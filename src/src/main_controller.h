#pragma once

#include "board_controller.h"
#include "menu.h"
#include "menu_option.h"
#include "game_state.h"
#include "menu_option_key.h"

class main_controller
{
private:
    board_controller bc;
    game_state state = game_state::menus;
    int selected_index = 0;
    bn::sprite_text_generator text_generator = bn::sprite_text_generator(gj::fixed_32x64_sprite_font);
    bn::sprite_palette_ptr palette_highlight = gj::fixed_32x64_sprite_font.item().palette_item().create_palette();
    bn::sprite_palette_ptr palette_grey = create_palette(16, 16, 16);
    bn::vector<bn::vector<bn::sprite_ptr, LONGEST_OPTION_TEXT>, MAX_OPTIONS_PER_SCREEN> menu_options_sprites;
    menu<menu_option_key> main_menu = menu<menu_option_key>("GEMMA");    // TODO: Think of name for game.
    menu<menu_option_key> play_menu = menu<menu_option_key>("PLAY", &main_menu);
    menu<menu_option_key> pause_menu = menu<menu_option_key>("PAUSE");
    menu<menu_option_key>* current_menu = &main_menu;
public:
    main_controller()
    {
        bc.hide();
        text_generator.set_center_alignment();

        // Init vectors inside menu_options_sprites.
        for (int i = 0; i < MAX_OPTIONS_PER_SCREEN; ++i)
        {
            menu_options_sprites.push_back(bn::vector<bn::sprite_ptr, LONGEST_OPTION_TEXT>());
        }

        main_menu.options.push_back(menu_option("PLAY", menu_option_key::play));
        main_menu.options.push_back(menu_option("RANKS", menu_option_key::ranks));
        main_menu.options.push_back(menu_option("SETTINGS", menu_option_key::settings));
        
        play_menu.options.push_back(menu_option("SPRINT", menu_option_key::sprint));
        play_menu.options.push_back(menu_option("TIME ATTACK", menu_option_key::time_attack));

        pause_menu.options.push_back(menu_option("RESUME", menu_option_key::resume));
        pause_menu.options.push_back(menu_option("RESTART", menu_option_key::restart));
        pause_menu.options.push_back(menu_option("QUIT", menu_option_key::quit));
    }

    void update()
    {
        if (state == game_state::menus)
        {
            // TODO: Can optimise by only redrawing sprites when menu is different from last frame.
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

            if (bn::keypad::up_pressed() && selected_index > 0)
            {
                selected_index -= 1;
            }
            else if (bn::keypad::down_pressed() && selected_index < (current_menu->options.size() - 1))
            {
                selected_index += 1;
            }
            else if (bn::keypad::start_pressed() && current_menu == &pause_menu)    // Start button shortcut for resume.
            {
                // TODO: This code is repeated from `resume` case below.
                state = game_state::ingame;
                bc.show();

                for (auto o : menu_options_sprites)
                {
                    for (auto s : o)
                    {
                        s.set_visible(false);
                    }
                }
            }
            else if (bn::keypad::a_pressed())
            {
                auto key = current_menu->options[selected_index].key;
                
                if (key == menu_option_key::play)
                {
                    current_menu = &play_menu;
                    selected_index = 0;
                }
                else if (key == menu_option_key::sprint)
                {
                    bc.reset();
                    bc.show();
                    state = game_state::ingame;

                    for (auto o : menu_options_sprites)
                    {
                        for (auto s : o)
                        {
                            s.set_visible(false);
                        }
                    }
                }
                else if (key == menu_option_key::resume)
                {
                    state = game_state::ingame;
                    bc.show();

                    for (auto o : menu_options_sprites)
                    {
                        for (auto s : o)
                        {
                            s.set_visible(false);
                        }
                    }
                }
                else if (key == menu_option_key::restart)
                {
                    bc.reset();
                    bc.show();
                    state = game_state::ingame;

                    for (auto o : menu_options_sprites)
                    {
                        for (auto s : o)
                        {
                            s.set_visible(false);
                        }
                    }
                }
                else if (key == menu_option_key::quit)
                {
                    state = game_state::menus;
                    current_menu = &main_menu;
                }
            }
            else if (bn::keypad::b_pressed())
            {
                if (current_menu->previous_menu != nullptr)
                {
                    current_menu = current_menu->previous_menu;
                }
            }
        }
        else if (state == game_state::ingame)
        {
            bc.update();

            if (bn::keypad::start_pressed())
            {
                bc.hide();
                current_menu = &pause_menu;
                selected_index = 0;
                state = game_state::menus;
            }
        }
        //else if (state == game_state::paused)
        //{
        //    if (bn::keypad::start_pressed())
        //    {
        //        bc.show();
        //        state = game_state::ingame;
        //    }
        //}
        else
        {
            BN_ASSERT(false, "Invalid game state");
        }

        bn::core::update();
    }
};