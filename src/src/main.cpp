#include "bn_bg_palettes.h"
#include "bn_core.h"
#include "board_controller.h"
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

constexpr int LONGEST_OPTION_TEXT = 8;
constexpr int MAX_OPTIONS_PER_SCREEN = 3;
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

    menu_options_collection main_menu_options;
    main_menu_options.push_back(menu_option("PLAY", menu_option_key::play));
    main_menu_options.push_back(menu_option("RANKS", menu_option_key::ranks));
    main_menu_options.push_back(menu_option("SETTINGS", menu_option_key::settings));

    menu_options_collection play_menu_options;
    play_menu_options.push_back(menu_option("SPRINT", menu_option_key::sprint));
    play_menu_options.push_back(menu_option("TIME ATTACK", menu_option_key::time_attack));

    menu_options_collection& current_menu_options = main_menu_options;

    while (true)
    {
        //bc.update();

        if (bn::keypad::up_pressed())
        {
            selected_index -= 1;
        }
        else if (bn::keypad::down_pressed())
        {
            selected_index += 1;
        }
        else if (bn::keypad::a_pressed())
        {
            if (current_menu_options[selected_index].key == menu_option_key::play)
            {
                current_menu_options = play_menu_options;
                selected_index = 0;
            }
        }

        for (int i = 0; i < current_menu_options.max_size(); ++i)
        {
            menu_options_sprites[i].clear();
        }

        for (int i = 0; i < current_menu_options.size(); ++i)
        {
            text_generator.generate(0, -20 + (i * 20), current_menu_options[i].text, menu_options_sprites[i]);

            auto palette = i == selected_index ? palette_highlight : palette_grey;
            for (auto s : menu_options_sprites[i])
            {
                s.set_palette(palette);
            }
        }

        bn::core::update();
    }
}