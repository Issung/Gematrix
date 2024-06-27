#pragma once

#include "game_controller.h"
#include "menu.h"
#include "menu_option.h"
#include "game_state.h"
#include "menu_option_key.h"
#include "memory.h"

class main_controller
{
private:
    game_controller bc;
    game_state state = game_state::menus;
    int selected_index = 0;
    bn::sprite_text_generator text_generator = bn::sprite_text_generator(gj::fixed_32x64_sprite_font);
    bn::sprite_palette_ptr palette_highlight = gj::fixed_32x64_sprite_font.item().palette_item().create_palette();
    bn::sprite_palette_ptr palette_grey = create_palette(16, 16, 16);
    bn::vector<bn::sprite_ptr, LONGEST_OPTION_TEXT> menu_title_sprites;
    bn::vector<bn::vector<bn::sprite_ptr, LONGEST_OPTION_TEXT>, MAX_OPTIONS_PER_SCREEN> menu_options_sprites;
    menu main_menu = menu("GEMMA");    // TODO: Think of name for game.
    menu play_menu = menu("PLAY", &main_menu);
    menu ranks_menu = menu("RANKS", &main_menu);
    menu settings_menu = menu("SETTINGS", &main_menu);  // [0] = SFX, [1] = MUSIC
    menu pause_menu = menu("PAUSE");
    menu* current_menu = &main_menu;

    void generate_options_text()
    {
        for (int i = 0; i < current_menu->options.max_size(); ++i)
        {
            menu_options_sprites[i].clear();
        }

        for (int i = 0; i < current_menu->options.size(); ++i)
        {
            text_generator.generate(0, -20 + (i * 20), current_menu->options[i].text, menu_options_sprites[i]);
        }
    }

    // Set to `nullptr` to hide menu.
    void change_menu(menu* new_menu)
    {
        current_menu = new_menu;
        selected_index = 0;

        if (new_menu == nullptr)
        {
            menu_title_sprites.clear();
            for (auto o : menu_options_sprites)
            {
                for (auto s : o)
                {
                    s.set_visible(false);
                }
            }
        }
        else
        {
            // Title
            menu_title_sprites.clear();
            text_generator.generate(0, -50, current_menu->title, menu_title_sprites);

            // Options
            generate_options_text();
        }
    }

    void change_state(game_state new_state)
    {
        state = new_state;

        if (state == game_state::menus)
        {
            change_menu(&main_menu);
            bc.hide();
        }
        else if (state == game_state::paused)
        {
            if (bn::music::playing()) bn::music::pause();
            bn::sound_items::pause.play();
            bc.hide();
            change_menu(&pause_menu);
        }
        else if (state == game_state::ingame)
        {
            if (bn::music::paused()) bn::music::resume();
            change_menu(nullptr);
            bc.show();
        }
    }

    void update_menus()
    {
        for (int i = 0; i < current_menu->options.size(); ++i)
        {
            auto palette = i == selected_index ? palette_highlight : palette_grey;
            for (auto s : menu_options_sprites[i])
            {
                s.set_palette(palette);
            }
        }

        if (bn::keypad::up_pressed() && selected_index > 0)
        {
            selected_index -= 1;
            bn::sound_items::menu_up.play();
        }
        else if (bn::keypad::down_pressed() && selected_index < (current_menu->options.size() - 1))
        {
            selected_index += 1;
            bn::sound_items::menu_down.play();
        }
        else if (bn::keypad::start_pressed() && state == game_state::paused)    // Start button shortcut for resume when paused.
        {
            change_state(game_state::ingame);
        }
        else if (bn::keypad::a_pressed())
        {
            auto key = current_menu->options[selected_index].key;
            
            // TODO: Pull this long if-else chain out into its own function/switch block.
            
            // MENU: MAIN
            if (key == menu_option_key::play)
            {
                change_menu(&play_menu);
                bn::sound_items::menu_ok.play();
            }
            else if (key == menu_option_key::ranks)
            {
                change_menu(&ranks_menu);
                bn::sound_items::menu_ok.play();
            }
            else if (key == menu_option_key::settings)
            {
                change_menu(&settings_menu);
                bn::sound_items::menu_ok.play();
            }

            // MENU: PLAY
            else if (key == menu_option_key::sprint)
            {
                // TODO: Make game modes do something different.
                // TODO: Make sub-menus for sprint/timeattack to select goal score / time limit.
                bc.newgame_sprint(50);
                change_state(game_state::ingame);
            }
            else if (key == menu_option_key::time_attack)
            {
                bc.newgame_timeattack(5);
                change_state(game_state::ingame);
            }

            // MENU: SETTINGS
            else if (key == menu_option_key::sfx_toggle)
            {
                auto new_setting = !memory::sfx_enabled();
                memory::sfx_enabled_set(new_setting);
                memory::save();
                settings_menu.options[0].text = new_setting ? "SFX ON" : "SFX OFF";
                generate_options_text();
            }
            else if (key == menu_option_key::music_toggle)
            {
                auto new_setting = !memory::music_enabled();
                memory::music_enabled_set(new_setting);
                memory::save();
                settings_menu.options[1].text = new_setting ? "MUSIC ON" : "MUSIC OFF";
                generate_options_text();
            }

            // MENU: PAUSE
            else if (key == menu_option_key::resume)
            {
                change_state(game_state::ingame);
            }
            else if (key == menu_option_key::restart)
            {
                bn::music::stop();
                bc.reset();
                change_state(game_state::ingame);
            }
            else if (key == menu_option_key::quit)
            {
                change_state(game_state::menus);
                bn::music::stop();
                bn::sound_items::menu_ok.play();
            }
        }
        else if (bn::keypad::b_pressed())
        {
            if (current_menu->previous_menu != nullptr)
            {
                change_menu(current_menu->previous_menu);
                bn::sound_items::menu_back.play();
            }
        }
    }

public:
    void update()
    {
        if (state == game_state::ingame)
        {
            if (bn::keypad::start_pressed())
            {
                change_state(game_state::paused);
            }
            else
            {
                bool game_done = bc.update();

                if (game_done)
                {
                    change_state(game_state::menus);
                }
            }
        }
        else if (state == game_state::menus || state == game_state::paused)
        {
            update_menus();
        }
        else
        {
            BN_ASSERT(false, "Invalid game state");
        }

        bn::core::update();
    }

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
        play_menu.options.push_back(menu_option("SURVIVAL", menu_option_key::survival));

        settings_menu.options.push_back(menu_option(memory::sfx_enabled() ? "SFX ON" : "SFX OFF", menu_option_key::sfx_toggle));
        settings_menu.options.push_back(menu_option(memory::music_enabled() ? "MUSIC ON" : "MUSIC OFF", menu_option_key::music_toggle));

        pause_menu.options.push_back(menu_option("RESUME", menu_option_key::resume));
        pause_menu.options.push_back(menu_option("RESTART", menu_option_key::restart));
        pause_menu.options.push_back(menu_option("QUIT", menu_option_key::quit));

        change_menu(&main_menu);
    }
};