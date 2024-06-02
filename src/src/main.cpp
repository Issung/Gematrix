#include "bn_bg_palettes.h"
#include "bn_core.h"
#include "board_controller.h"
#include "menu.h"
#include "menu_option.h"

enum menu_option_key
{
    // Main Menu
    play,
    ranks,
    settings,

    // Play Menu
    sprint,
    time_attack,
};

#define menu_options_collection bn::vector<menu_option<menu_option_key>, MAX_OPTIONS_PER_SCREEN>

int main()
{
    bn::core::init();
    bn::bg_palettes::set_transparent_color(bn::color(0, 0, 0));

    int selected_index = 0;
    board_controller bc;
    bc.hide();

    // TODO: Make menu option classes that have the sprites and an enum for their selection action.
    // Then a menu can be a list of these option classes to be easily displayed and swapped between.
    auto text_generator = bn::sprite_text_generator(gj::fixed_32x64_sprite_font);
    text_generator.set_center_alignment();
    auto palette_highlight = gj::fixed_32x64_sprite_font.item().palette_item().create_palette();
    auto palette_grey = create_palette(16, 16, 16);

    bn::vector<bn::vector<bn::sprite_ptr, LONGEST_OPTION_TEXT>, MAX_OPTIONS_PER_SCREEN> menu_options_sprites;
    for (int i = 0; i < MAX_OPTIONS_PER_SCREEN; i++)    // Initialise the array.
    {
        menu_options_sprites.push_back(bn::vector<bn::sprite_ptr, LONGEST_OPTION_TEXT>());
    }

    auto main_menu = menu<menu_option_key>("GEMMA");
    main_menu.options.push_back(menu_option("PLAY", menu_option_key::play));
    main_menu.options.push_back(menu_option("RANKS", menu_option_key::ranks));
    main_menu.options.push_back(menu_option("SETTINGS", menu_option_key::settings));

    auto play_menu = menu<menu_option_key>("PLAY", &main_menu);
    play_menu.options.push_back(menu_option("SPRINT", menu_option_key::sprint));
    play_menu.options.push_back(menu_option("TIME ATTACK", menu_option_key::time_attack));

    auto current_menu = &main_menu;

    while (true)
    {
        // TODO: Call this when in-game.
        //bc.update();

        if (bn::keypad::up_pressed() && selected_index > 0)
        {
            selected_index -= 1;
        }
        else if (bn::keypad::down_pressed() && selected_index < (current_menu->options.size() - 1))
        {
            selected_index += 1;
        }
        else if (bn::keypad::a_pressed())
        {
            auto key = current_menu->options[selected_index].key;
            
            if (key == menu_option_key::play)
            {
                current_menu = &play_menu;
                selected_index = 0;
            }
        }
        else if (bn::keypad::b_pressed())
        {
            if (current_menu->previous_menu != nullptr)
            {
                current_menu = current_menu->previous_menu;
            }
        }

        for (int i = 0; i < current_menu->options.max_size(); ++i)
        {
            menu_options_sprites[i].clear();
        }

        for (int i = 0; i < current_menu->options.size(); ++i)
        {
            text_generator.generate(0, -20 + (i * 20), current_menu->options[i].text, menu_options_sprites[i]);

            auto palette = i == selected_index ? palette_highlight : palette_grey;
            for (auto s : menu_options_sprites[i])
            {
                s.set_palette(palette);
            }
        }

        bn::core::update();
    }
}