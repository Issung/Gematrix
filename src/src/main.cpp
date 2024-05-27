#include "bn_bg_palettes.h"
#include "bn_core.h"
#include "board_controller.h"

int main()
{
    bn::core::init();
    bn::bg_palettes::set_transparent_color(bn::color(0, 0, 0));

    board_controller bc;

    while (true)
    {
        bc.update();
        bn::core::update();
    }
}