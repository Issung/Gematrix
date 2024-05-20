#ifndef BOARD
#define BOARD

#include "bn_vector.h"
#include "bn_random.h"

enum class gem_type
{
    Red,
    Green,
    Blue,
    Purple,
    Orange,
    Wildcard
};

class board
{
public: // Public fields
    static const int rows = 5;
    static const int cols = 6;
    static const int total_gems = rows * cols; // Total gems on the board (rows * cols).
    static const int max_colors = 6;   // The amount of colors.
    gem_type gems[rows][cols];
private:    // Private fields
    bn::random rand = bn::random();
    static const int match_max_length = 5;
public:
    void new_board()
    {
        for (int r = 0; r < board::rows; r++)
        {
            for (int c = 0; c < board::cols; c++)
            {
                auto val = rand.get_int(board::max_colors);
                gems[r][c] = (gem_type)val;
            }
        }
    }

    // Return a list of lists of indexes of matches.
    //bn::vector<bn::vector<int, 5>, 2> get_matches(bn::ivector<gem_type>& row)
    bn::vector<bn::vector<int, 5>, 2> get_matches(gem_type row[], int length)
    {
        auto matches = bn::vector<bn::vector<int, 5>, 2>();    // Max of 2 matches.
        auto currentSequence = bn::vector<gem_type, 5>();   // Maximum match length is 5(?).
        //auto length = row.size();

        for (int i = 0; i < length; i++)
        {
            auto lastSequence = bn::vector<gem_type, 5>(currentSequence);   // Copy the current sequence before we modify it.
            bool sequenceCouldMatch = sequence_can_match(lastSequence);
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
    bool sequence_is_match(bn::ivector<gem_type>& gems)
    {
        // Need atleast 3 gems to match.
        if (gems.size() < 3)
        {
            return false;
        }

        return sequence_can_match(gems);
    }

    // True/false does this sequence of gems have any gems that cannot match with eachother.
    bool sequence_can_match(bn::ivector<gem_type>& gems)
    {
        // Check that each gem can match with every other gem.
        // TODO: This can be further optimised by not repeating checks.

        auto length = gems.size();

        for (int i = 0; i < length; i++)
        {
            for (int j = 0; j < length; j++)
            {
                if (i != j)
                {
                    if (!gems_match(gems[i], gems[j]))
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

    //bn::ivector<int> range(int start, int count)
    bn::vector<int, match_max_length> range(int start, int count)
    {
        auto list = bn::vector<int, match_max_length>();

        for (int i = start; i < start+count; ++i)
        {
            list.push_back(i);
        }

        return list;
    }
};

#endif