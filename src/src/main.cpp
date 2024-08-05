#include "bn_bg_palettes.h"
#include "bn_core.h"
#include "main_controller.h"
#include "memory.h"
#include "background_controller.h"
#include "bn_bg_palette_ptr.h"
#include "bn_regular_bg_ptr.h"
#include "bn_regular_bg_items_splash.h"
#include "bn_optional.h"
#include "bn_blending.h"

int main()
{
    bn::core::init();
    memory::init();
    auto black = bn::color(0, 0, 0);
    bn::bg_palettes::set_transparent_color(black);

    bn::fixed alpha = 0;
    bn::fixed inc = 0.0333333333;
    auto bg = bn::regular_bg_items::splash.create_bg_optional(8, 48);
    bg->set_priority(0);
    bg->set_blending_enabled(true);
    bn::blending::set_transparency_alpha(alpha);
    auto palette = bn::optional<bn::bg_palette_ptr>(bg->palette());

    // Fade in.
    for (int i = 0; i < 30; ++i)
    {
        alpha += inc;
        bn::blending::set_transparency_alpha(alpha);
        bn::core::update();
    }
    bn::blending::set_transparency_alpha(1);

    // Pause.
    for (int i = 0; i < 240; ++i)   // 4 seconds max.
    {
        if (i >= 30 && bn::keypad::any_pressed())   // Half a second minimum on the full opacity splash.
        {
            break;
        }

        bn::core::update();
    }

    // Build the game controllers now, so they are visible when fading out.
    auto bgc = background_controller();
    auto controller = main_controller(bgc);

    // Fade out.
    for (int i = 0; i < 30; ++i)
    {
        alpha -= inc;
        bn::blending::set_transparency_alpha(alpha);

        bgc.update();
        controller.mini_update();

        bn::core::update();
    }

    // Fade-to-menu complete, release the bg & palette pointers.
    bg.reset();
    palette.reset();

    while (true)
    {
        bgc.update();
        controller.update();

        bn::core::update();
    }
}