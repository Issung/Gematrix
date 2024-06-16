#ifndef BOARD
#define BOARD

#include "bn_vector.h"
#include "bn_random.h"
#include "match.h"
#include "gem_type.h"

#define match_collection bn::vector<match, 20>
#define gem_drops_collection bn::vector<gem_drop, board::total_gems>

class gem_drop
{
public:
    int8_t col;
    int8_t from_row;
    int8_t to_row;

    gem_type type;

    gem_drop(
        int8_t _col,
        int8_t _from_row, int8_t _to_row,
        gem_type _type
    ) : col(_col),
        from_row(_from_row), to_row(_to_row),
        type(_type)
    {

    }
};

// Class responsible for handling board logic.
class board
{
public: // Public fields
    static const int rows = 5;
    static const int cols = 6;
    static const int row_length = cols;
    static const int col_length = rows;
    static const int total_gems = rows * cols; // Total gems on the board (rows * cols).
    static const int max_colors = 6;   // The amount of colors.
    gem_type gems[rows][cols];  // [row][col]
private:    // Private fields
    bn::random rand = bn::random();

    gem_type gen_new_gem()
    {
        return (gem_type)rand.get_int(board::max_colors - 1); // TODO: No wildcards for now
    }
public: 
    board()
    {
        new_board();
    }

    // TODO: Return collection of gem drops and get board_drawer to animate it.
    // TODO: Generate a board that has no pre-existing matches.
    void new_board()
    {
        while (true)
        {
            for (int r = 0; r < board::rows; r++)
            {
                for (int c = 0; c < board::cols; c++)
                {
                    auto val = gen_new_gem();
                    gems[r][c] = val;
                    BN_LOG("Generated gem ", r, ",", c, ": ", (uint8_t)val);
                }
            }

            // OPTIMISATION: Can be improved by instead of regenerating the board, just replace the matches with random gems until the match is broken.
            if (get_all_matches().size() == 0)
            {
                break;
            }
        }
    }

    void swap(int row_a, int col_a, int row_b, int col_b)
    {
        auto gem_a = gems[row_a][col_a];
        auto gem_b = gems[row_b][col_b];
        gems[row_a][col_a] = gem_b;
        gems[row_b][col_b] = gem_a;
    }

    // TODO: If we're deleting matches after a swap we can optimise by only checking rows/columns affected by the swap.
    match_collection delete_matches()
    {
        match_collection matches = get_all_matches();

        for (auto m : matches)
        {
            for (auto pos : m.positions)
            {
                gems[pos.row][pos.col] = gem_type::Empty;
            }
        }

        return matches;
    }

    // "Drop" gems into empty spots, and generate new gems for empty spots at the top.
    // Returns a list of drops that were performed.
    gem_drops_collection drop_gems()
    {
        auto drops = gem_drops_collection();

        for (int col = 0; col < cols; ++col)
        {
            // Track how many new gems are created in each column for determining the drop height of new gems.
            int newGemsInCol = 0;

            for (int row = rows - 1; row >= 0; --row)
            {
                if (gems[row][col] == gem_type::Empty)
                {
                    auto didPullGemDown = false;
                    
                    // Find the nearest non-empty gem above the empty spot.
                    for (int searchRow = row - 1; searchRow >= 0; --searchRow)
                    {
                        if (gems[searchRow][col] != gem_type::Empty)
                        {
                            // Move the found gem down to the empty spot
                            gems[row][col] = gems[searchRow][col];
                            gems[searchRow][col] = gem_type::Empty;
                            
                            // Create a drop record.
                            drops.push_back(gem_drop(col, searchRow, row, gems[row][col]));
                            didPullGemDown = true;
                            break;
                        }
                    }

                    // No gem was able to be pulled down, we need to generate a new one.
                    if (!didPullGemDown)
                    {
                        newGemsInCol += 1;

                        auto new_gem_type = gen_new_gem();
                        gems[row][col] = new_gem_type;
                        drops.push_back(gem_drop(col, -newGemsInCol, row, new_gem_type));
                    }
                }
            }
        }

        return drops;
    }

    void spawn_new_gems()
    {
        for (int r = 0; r < rows; ++r)
        {
            for (int c = 0; c < cols; ++c)
            {
                if (gems[r][c] == gem_type::Empty)
                {
                    gems[r][c] = gen_new_gem();
                }
            }
        }
    }

    // Find all matches on the board.
    match_collection get_all_matches()
    {
        int r[rows] { 0, 1, 2, 3, 4 };
        int c[cols] { 0, 1, 2, 3, 4, 5 };
        return get_matches_in(r, rows, c, cols);
    }

    // Find matches in provided row and match indices.
    match_collection get_matches_in(
        int row_indices[], int rows_length, // Indices of rows to check.
        int col_indices[], int cols_length  // Indices of cols to check.
    )
    {
        auto matches = match_collection();

        for (int i = 0; i < rows_length; ++i)
        {
            auto row = row_indices[i];
            auto matches_in_row = get_matches_in_sequence(gems[row], row_length);

            for (int match_index = 0; match_index < matches_in_row.size(); ++match_index)
            {
                auto match_type = gem_type::Empty;
                auto positions = bn::vector<match_position, match::MAX_LENGTH>();

                for (int position_index = 0; position_index < matches_in_row[match_index].size(); ++position_index)
                {
                    auto col = matches_in_row[match_index][position_index];
                    positions.push_back(match_position(row, col));

                    if (gems[row][col] != gem_type::Wildcard)
                    {
                        match_type = gems[row][col];
                    }
                }

                matches.push_back(match(match_type, positions));
            }
        }

        for (int i = 0; i < cols_length; ++i)
        {
            auto col = col_indices[i];
            gem_type col_gems[col_length];

            // Build a "row" array from the column.
            for (int j = 0; j < col_length; ++j)
            {
                col_gems[j] = gems[j][col];
            }

            auto matches_in_col = get_matches_in_sequence(col_gems, col_length);

            for (int match_index = 0; match_index < matches_in_col.size(); ++match_index)
            {
                auto match_type = gem_type::Empty;
                auto positions = bn::vector<match_position, match::MAX_LENGTH>();

                for (int position_index = 0; position_index < matches_in_col[match_index].size(); ++position_index)
                {
                    auto row = matches_in_col[match_index][position_index];
                    positions.push_back(match_position(row, col));

                    if (gems[row][col] != gem_type::Wildcard)
                    {
                        match_type = gems[row][col];
                    }
                }

                matches.push_back(match(match_type, positions));
            }
        }

        return matches;
    }

    // The max length of a sequence is the length of a maximum match + 1 because the sequence will then be broken.
    // E.g. [BBBBBG] (5 blues starting from the left and one green, the sequence will be broken and [BBBBB] will be returned.)
    static const int max_sequence_length = match::MAX_LENGTH + 1;

    // The maximum amount of matches that can occur in a sequence.
    // Rows are the longest. E.g. in a row (length 6) [BBBGGG].
    static const int max_matches_in_a_sequence = 2;
    
    // Return a list of lists of indexes of matches.
    // bn::vector<bn::vector<int, 5>, 2> get_matches(bn::ivector<gem_type>& row)
    // TODO: Move to be private.
    bn::vector<bn::vector<int, max_sequence_length>, max_matches_in_a_sequence> get_matches_in_sequence(gem_type row[], int length)
    {
        auto matches = bn::vector<bn::vector<int, max_sequence_length>, max_matches_in_a_sequence>();
        auto currentSequence = bn::vector<gem_type, max_sequence_length>();
        //auto length = row.size();

        for (int i = 0; i < length; i++)
        {
            auto lastSequence = bn::vector<gem_type, max_sequence_length>(currentSequence);   // Copy the current sequence before we modify it.
            //bool sequenceCouldMatch = sequence_can_match(lastSequence);   // Unused
            bool sequenceHadMatch = sequence_is_match(lastSequence);

            currentSequence.push_back(row[i]);
            auto sequenceCanMatch = sequence_can_match(currentSequence);
            auto sequenceHasMatch = sequence_is_match(currentSequence);

            auto matchLost = sequenceHadMatch && !sequenceCanMatch;  // Did the read-along turn from a match into a no-match.
            auto isLastGem = i == length - 1;   // Is this the last iteration.

            if (isLastGem && sequenceHasMatch)
            {
                auto indexes = range(i - currentSequence.size() + 1, currentSequence.size());
                matches.push_back(indexes);
            }
            else if (matchLost)
            {
                auto indexes = range(i - lastSequence.size(), lastSequence.size());
                matches.push_back(indexes);
            }

            // If the current sequence can't be a match, restart the sequence list.
            if (!sequenceCanMatch)
            {
                currentSequence.clear();

                // Special case, if the last gem of the last sequence was a wildcard, we must carry it over so it can be
                // used as the start of the next sequence.
                auto lastSequenceLastElement = *lastSequence.end();
                if (lastSequenceLastElement == gem_type::Wildcard)
                {
                    currentSequence.push_back(lastSequenceLastElement);
                }

                currentSequence.push_back(row[i]);
            }
        }

        return matches;
    }

private:
    // True/false is this sequence a match.
    bool sequence_is_match(bn::ivector<gem_type>& sequence)
    {
        // Need atleast 3 gems to match.
        if (sequence.size() < 3)
        {
            return false;
        }

        return sequence_can_match(sequence);
    }

    // True/false does this sequence of gems have any gems that cannot match with eachother.
    bool sequence_can_match(bn::ivector<gem_type>& sequence)
    {
        // Check that each gem can match with every other gem.
        // TODO: This can be further optimised by not repeating checks.

        auto length = sequence.size();

        for (int i = 0; i < length; i++)
        {
            for (int j = 0; j < length; j++)
            {
                if (i != j)
                {
                    if (!gems_match(sequence[i], sequence[j]))
                    {
                        return false;
                    }
                }
            }
        }

        return true;
    }

    bool gems_match(gem_type a, gem_type b)
    {
        // Empties can't match.
        // TODO: This should eventually become redundant, then remove.
        if (a == gem_type::Empty || b == gem_type::Empty)
        {
            return false;
        }

        // Wildcard cannot match with wildcard (max 1 wildcard per match).
        if (a == gem_type::Wildcard && b == gem_type::Wildcard)
        {
            return false;
        }
        
        // Types must be equal, if there is no wildcard.
        if (a != b && a != gem_type::Wildcard && b != gem_type::Wildcard)
        {
            return false;
        }
        
        return true;
    }

    bn::vector<int, match::MAX_LENGTH> range(int start, int count)
    {
        BN_ASSERT(count <= match::MAX_LENGTH, "A range should never be made of length greater than ", match::MAX_LENGTH);
        auto list = bn::vector<int, match::MAX_LENGTH>();

        for (int i = start; i < start + count; ++i)
        {
            list.push_back(i);
        }

        return list;
    }
};

#endif