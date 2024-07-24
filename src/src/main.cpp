#include "bn_bg_palettes.h"
#include "bn_core.h"
#include "main_controller.h"
#include "memory.h"
#include "bn_affine_bg_items_grid.h"
#include "bn_affine_bg_ptr.h"

int main()
{
    bn::core::init();
    memory::init();
    bn::bg_palettes::set_transparent_color(bn::color(0, 0, 0));

    // Make this before making the main_controller (which makes all other controllers)
    // So that the board background drawers over the top of this background.
    bn::affine_bg_ptr red_bg = bn::affine_bg_items::grid.create_bg(0, 0);

    auto controller = main_controller();

    while (true)
    {
        red_bg.set_x(red_bg.x() + 0.1);
        red_bg.set_y(red_bg.y() + 0.05);
        controller.update();
    }
}