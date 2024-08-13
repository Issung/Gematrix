#pragma once

#include "memory.h"
#include "bn_fixed.h"
#include "bn_random.h"
#include "bn_affine_bg_items_grid.h"
#include "bn_affine_bg_ptr.h"
#include "bn_bg_palette_ptr.h"

// Construct this before the `main_controller` (which makes all other controllers)
// so that the board background drawers over the top of this background.
class background_controller
{
private:
    const bn::fixed SPEED_MAX = 15.0;
    const bn::fixed SPEED_MIN_DEFAULT = 1.0;
    const bn::fixed SPEED_DECREASE = 0.125;
    const bn::fixed BUMP = 0.35;
    const bn::fixed OFFSET_MAX = 0.1;
    bn::random rand;
    bn::affine_bg_ptr bg = bn::affine_bg_items::grid.create_bg(0, 0);
    bn::fixed x_offset = 0.1;
    bn::fixed y_offset = 0.05;
    bn::fixed speed_min = SPEED_MIN_DEFAULT;
    bn::fixed speed = 1.0;
    bool braking = false;
    bool frozen = false;
public:
    void thaw()
    {
        frozen = false;
    }

    void freeze()
    {
        frozen = true;
    }

    // Apply brakes to the background speed, slowing it to a halt until `reset()`.
    void brake()
    {
        speed_min = 0.0;
        braking = true;
    }

    // Reset speed, randomize a new direction, and unfreeze animation.
    void reset()
    {
        braking = false;
        frozen = false;
        speed_min = SPEED_MIN_DEFAULT;
        speed = speed_min;
        x_offset = rand.get_fixed(-OFFSET_MAX, OFFSET_MAX);
        y_offset = rand.get_fixed(-OFFSET_MAX, OFFSET_MAX);
    }

    // Accelerate the background scrolling speed, use for game events to build a feeling of momentum.
    void accelerate()
    {
        if (braking)
        {
            return;
        }
        
        speed += BUMP;

        if (speed < SPEED_MAX)
        {
            speed = SPEED_MAX;
        }
    }

    void update()
    {
        if (frozen)
        {
            return;
        }

        if (speed > speed_min)
        {
            speed -= SPEED_DECREASE;

            if (speed < speed_min)
            {
                speed = speed_min;
            }
        }

        bg.set_x(bg.x() + (x_offset * speed));
        bg.set_y(bg.y() + (y_offset * speed));
    }

    // Update palette of the background to match the user's setting.
    void update_palette()
    {
        auto palette = bg.palette();
        
        // The background palette is shared across the background, 

        if (memory::palette() == palette_setting::og)
        {
            palette.set_color(240, bn::color(1, 1, 1));
            palette.set_color(241, bn::color(2, 2, 2));
            palette.set_color(242, bn::color(3, 3, 3));
            palette.set_color(243, bn::color(4, 4, 4));
            palette.set_color(244, bn::color(5, 5, 5));
        }
        else
        {
            palette.set_color(240, bn::color(6, 6, 6));
            palette.set_color(241, bn::color(9, 9, 9));
            palette.set_color(242, bn::color(12, 12, 12));
            palette.set_color(243, bn::color(15, 15, 15));
            palette.set_color(244, bn::color(18, 18, 18));
        }
    }

    // Constructor.
    background_controller()
    {
        update_palette();
    }
};