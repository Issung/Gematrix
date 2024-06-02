#include "bn_bg_palettes.h"
#include "bn_core.h"
#include "main_controller.h"

int main()
{
    bn::core::init();
    bn::bg_palettes::set_transparent_color(bn::color(0, 0, 0));

    auto controller = main_controller();

    while(true)
    {
        controller.update();
    }
}