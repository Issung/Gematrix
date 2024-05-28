#include "bn_bg_palettes.h"
#include "bn_core.h"
#include "board_controller.h"

int main()
{
    bn::core::init();
    bn::bg_palettes::set_transparent_color(bn::color(0, 0, 0));

    int index = 0;
    board_controller bc;
    bc.hide();

    // TODO: Make menu option classes that have the sprites and an enum for their selection action.
    // Then a menu can be a list of these option classes to be easily displayed and swapped between.
    auto text_generator = bn::sprite_text_generator(gj::fixed_32x64_sprite_font);
    text_generator.set_center_alignment();
    bn::vector<bn::sprite_ptr, 4> menutext_sprites_play;
    bn::vector<bn::sprite_ptr, 5> menutext_sprites_ranks;
    bn::vector<bn::sprite_ptr, 7> menutext_sprites_settings;
    text_generator.generate(0, -20, "PLAY", menutext_sprites_play);
    text_generator.generate(0, -00, "RANKS", menutext_sprites_ranks);
    text_generator.generate(0, +20, "OPTIONS", menutext_sprites_settings);
    auto highlight_palette = menutext_sprites_play[0].palette();
    auto grey_palette = create_palette(16, 16, 16);

    bn::vector<bn::vector<bn::sprite_ptr, 7>, 3> menu_options;
    menu_options.push_back(menutext_sprites_play);
    menu_options.push_back(menutext_sprites_ranks);
    menu_options.push_back(menutext_sprites_settings);

    while (true)
    {
        //bc.update();

        if (bn::keypad::up_pressed())
        {
            index -= 1;
        }
        else if (bn::keypad::down_pressed())
        {
            index += 1;
        }
        else if (bn::keypad::a_pressed())
        {
            bc.show();
        }
        else if (bn::keypad::b_pressed())
        {
            bc.hide();
        }

        for (int i = 0; i < menu_options.size(); ++i)
        {
            auto palette = i == index ? highlight_palette : grey_palette;
            for (auto s : menu_options[i])
            {
                s.set_palette(palette);
            }
        }

        bn::core::update();
    }
}