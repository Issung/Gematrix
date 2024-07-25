#pragma once

#include "bn_fixed.h"
#include "bn_random.h"
#include "bn_affine_bg_items_grid.h"
#include "bn_affine_bg_ptr.h"

// Construct this before the `main_controller` (which makes all other controllers)
// so that the board background drawers over the top of this background.
class background_controller
{
private:
    const bn::fixed SPEED_MIN = 1.0;
    const bn::fixed SPEED_MAX = 15.0;
    const bn::fixed SPEED_DECREASE = 0.125;
    const bn::fixed BUMP = 0.35;
    const bn::fixed OFFSET_MAX = 0.1;
    bn::random rand;
    bn::affine_bg_ptr bg = bn::affine_bg_items::grid.create_bg(0, 0);
    bn::fixed x_offset = 0.1;
    bn::fixed y_offset = 0.05;
    bn::fixed speed = 1.0;

public:
    // Randomize the x/y background panning values.
    void randomize_direction()
    {
        x_offset = rand.get_fixed(-OFFSET_MAX, OFFSET_MAX);
        y_offset = rand.get_fixed(-OFFSET_MAX, OFFSET_MAX);
    }

    // Accelerate the background scrolling speed 
    void bump_speed()
    {
        speed += BUMP;

        if (speed < SPEED_MAX)
        {
            speed = SPEED_MAX;
        }
    }

    void update()
    {
        if (speed > SPEED_MIN)
        {
            speed -= SPEED_DECREASE;
        }

        bg.set_x(bg.x() + (x_offset * speed));
        bg.set_y(bg.y() + (y_offset * speed));
    }
};