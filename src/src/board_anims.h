#pragma once

#include "bn_array.h"
#include "bn_assert.h"
#include "board.h"

// An animation explaining the order of the gems, ints ranging from 0 - 29.
#define BOARD_ANIM bn::array<bn::array<int, board::cols>, board::rows>

// Generator for whole-board animations.
class board_anims
{
private:
    static bn::random rand; // Initialised below outside of the class.

    // Drop in one row at a time, from bottom to top, right to left.
    static BOARD_ANIM animate_drop_all_in()
    {
        BOARD_ANIM anim;

        int i = 0;
        for (int row = board::rows - 1; row >= 0; --row)
        {
            for (int col = board::cols - 1; col >= 0; --col)
            {
                anim[row][col] = i;
                ++i;
            }
        }

        return anim;
    }

    // Fill row by row, starting from left to right, then right to left, alternating to the top.
    static BOARD_ANIM animate_drop_all_in_alternating_rows()
    {
        int i = 0;
        BOARD_ANIM anim;

        for (int row = board::rows - 1; row >= 0; --row)
        {
            auto even = row % 2 == 0;
            for (int col = (even ? board::cols - 1 : 0); 
                even ? col >= 0 : col < board::cols; 
                even ? --col : ++col)
            {
                anim[row][col] = i;
                ++i;
            }
        }

        return anim;
    }

    // Drop gems into random columns until full.
    static BOARD_ANIM animate_scattered_drop_all_in()
    {
        int i = 0;
        BOARD_ANIM anim;
        int total_slots = board::rows * board::cols;
        bool filled[board::rows][board::cols] = {};
        int filled_count = 0;

        while (filled_count < total_slots)
        {
            int col = rand.get_int(board::cols);

            for (int row = board::rows - 1; row >= 0; --row)
            {
                if (filled[row][col] == false)
                {
                    filled[row][col] = true;
                    filled_count += 1;
                    anim[row][col] = i;
                    ++i;
                    break;
                }
            }
        }

        return anim;
    }

    // Fill row by row, dropping gems into random columns until full, moving up to the next row.
    static BOARD_ANIM animate_scattered_drop_all_in_row_by_row()
    {
        int i = 0;
        BOARD_ANIM anim;
        
        for (int row = board::rows - 1; row >= 0; --row)
        {
            bool filled[board::cols] = {};
            int filled_count = 0;

            while (filled_count < board::cols)
            {
                int col = rand.get_int(board::cols);

                if (filled[col] == false)
                {
                    filled[col] = true;
                    filled_count += 1;

                    anim[row][col] = i;
                    ++i;
                }
            }
        }

        return anim;
    }

public:
    // Fetch a randomly generated animation.
    static BOARD_ANIM get_random()
    {
        // Return constant test method here.

        int i = rand.get_int(4);
        
        if (i == 0) return animate_drop_all_in();
        else if (i == 1) return animate_drop_all_in_alternating_rows();
        else if (i == 2) return animate_scattered_drop_all_in();
        else if (i == 3) return animate_scattered_drop_all_in_row_by_row();
        else BN_ASSERT(false, "Invalid value for random all-drop-in animation.");

        return BOARD_ANIM();    // Return here will never be reached due to assert above, but we have to appease the compiler.
    }
};

bn::random board_anims::rand = bn::random();