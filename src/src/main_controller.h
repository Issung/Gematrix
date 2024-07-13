#pragma once

#include "game_controller.h"
#include "menu.h"
#include "menu_option.h"
#include "game_state.h"
#include "menu_option_key.h"
#include "memory.h"
#include "bn_string.h"
#include "highscore_entry_controller.h"
#include "game_mode.h"

class main_controller
{
private:
    game_controller gc;
    highscore_entry_controller hec;
    game_state state = game_state::menus;
    int selected_index = 0;
    bn::sprite_text_generator text_generator = bn::sprite_text_generator(gj::fixed_32x64_sprite_font);
    bn::sprite_palette_ptr palette_highlight = gj::fixed_32x64_sprite_font.item().palette_item().create_palette();
    bn::sprite_palette_ptr palette_grey = create_palette(16, 16, 16);
    bn::vector<bn::sprite_ptr, LONGEST_OPTION_TEXT> menu_title_sprites;
    bn::vector<bn::vector<bn::sprite_ptr, LONGEST_OPTION_TEXT>, MAX_OPTIONS_PER_SCREEN> menu_options_sprites;

    menu main_menu = menu("GEMMA");    // TODO: Think of name for game.
    menu play_menu = menu("PLAY", &main_menu);
    menu play_levels_menu = menu("LEVEL", &play_menu);  // After the user selects a mode on the `play_menu`, they select the level on this one.
    menu records_menu = menu("RECORDS", &main_menu);
    menu records_sprint_menu = menu("RECORDS (SPRINT)", &records_menu);
    menu records_timeattack_menu = menu("RECORDS (TIMEATTACK)", &records_menu);
    menu settings_menu = menu("SETTINGS", &main_menu);  // [0] = SFX, [1] = MUSIC
    menu pause_menu = menu("PAUSE");
    menu gameover_menu = menu("GAMEOVER", &main_menu);
    menu* current_menu = &main_menu;

    // The user's desired game mode when on the `play_levels_menu`.
    game_mode play_mode;
    bn::string_view sprint_levels_menu_title = "GOAL SCORE";
    bn::string_view timeattack_levels_menu_title = "TIME LIMIT";

    void generate_options_text()
    {
        for (int i = 0; i < current_menu->options.max_size(); ++i)
        {
            menu_options_sprites[i].clear();
        }

        // TODO: Position options nicely, and also obeying optional y position in menu.
        auto size = current_menu->get_options_count();
        auto y_pos = current_menu->get_y_position();
        for (int i = 0; i < size; ++i)
        {
            text_generator.generate(0, y_pos + (i * 20), current_menu->options[i].text, menu_options_sprites[i]);
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
            hec.hide();
            gc.hide();
        }
        else if (state == game_state::paused)
        {
            if (bn::music::playing()) bn::music::pause();
            bn::sound_items::pause.play();
            gc.hide();
            change_menu(&pause_menu);
        }
        else if (state == game_state::ingame)
        {
            if (bn::music::paused()) bn::music::resume();
            change_menu(nullptr);
            gc.show();
        }
        else if (state == game_state::hiscore)
        {
            change_menu(nullptr);
            gc.hide();

            auto record_position = memory::is_record(gc.get_mode(), gc.get_level(), gc.get_gamemode_metric());
            text_generator.generate(0, -50, "GAMEOVER", menu_title_sprites);
            
            if (record_position.has_value())
            {
                auto ordinal_position = util::ordinal_string(record_position.value());  // Position text (e.g. 1st, 3rd, etc).
                auto text = bn::format<menu_option::TEXT_MAX_LENGTH>("HISCORE! {}", ordinal_position);
                text_generator.generate(0, -30, text, menu_options_sprites[0]);
                auto score_str = gc.get_gamemode_metric_display_string();
                BN_LOG("SCORE STR: ", score_str);
                text_generator.generate(0, -15, score_str, menu_options_sprites[1]);
            }
        }
    }

    void update_menus()
    {
        //hec.update();
        //return;

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
            else if (key == menu_option_key::records)
            {
                change_menu(&records_menu);
                bn::sound_items::menu_ok.play();
            }
            else if (key == menu_option_key::settings)
            {
                change_menu(&settings_menu);
                bn::sound_items::menu_ok.play();
            }

            // MENU: PLAY
            else if (key == menu_option_key::play_sprint)
            {
                play_mode = game_mode::sprint;

                play_levels_menu.title = sprint_levels_menu_title;
                play_levels_menu.options.clear();
                play_levels_menu.options.push_back(menu_option(bn::to_string<5>(levels::sprint[0]), menu_option_key::play_level0));
                play_levels_menu.options.push_back(menu_option(bn::to_string<5>(levels::sprint[1]), menu_option_key::play_level1));
                play_levels_menu.options.push_back(menu_option(bn::to_string<5>(levels::sprint[2]), menu_option_key::play_level2));

                change_menu(&play_levels_menu);
            }
            else if (key == menu_option_key::play_timeattack)
            {
                play_mode = game_mode::timeattack;

                play_levels_menu.title = timeattack_levels_menu_title;
                play_levels_menu.options.clear();
                play_levels_menu.options.push_back(menu_option(util::frames_to_time_string(levels::timeattack[0]), menu_option_key::play_level0));
                play_levels_menu.options.push_back(menu_option(util::frames_to_time_string(levels::timeattack[1]), menu_option_key::play_level1));
                play_levels_menu.options.push_back(menu_option(util::frames_to_time_string(levels::timeattack[2]), menu_option_key::play_level2));

                change_menu(&play_levels_menu);
            }

            // MENU: PLAY LEVEL SELECT
            else if (key == menu_option_key::play_level0)
            {
                gc.newgame(play_mode, 0);
                change_state(game_state::ingame);
            }
            else if (key == menu_option_key::play_level1)
            {
                gc.newgame(play_mode, 1);
                change_state(game_state::ingame);
            }
            else if (key == menu_option_key::play_level2)
            {
                gc.newgame(play_mode, 2);
                change_state(game_state::ingame);
            }

            // MENU: RECORDS
            // TODO: All been broken by record levels
            /*else if (key == menu_option_key::records_sprint)
            {   
                records_sprint_menu.options.clear();
                for (int i = 0; i < MAX_RECORDS; ++i)
                {
                    auto record = memory::save_data.records_sprint[i];
                    auto name = bn::string<RECORD_NAME_LENGTH>(record.name.data(), RECORD_NAME_LENGTH);
                    auto str = bn::format<menu_option::TEXT_MAX_LENGTH>("{} {}", name, util::frames_to_time_millis_string(record.time_in_frames));
                    BN_LOG(str);
                    records_sprint_menu.options.push_back(menu_option(str, menu_option_key::noop));
                }
                change_menu(&records_sprint_menu);
            }
            else if (key == menu_option_key::records_timeattack)
            {   
                records_timeattack_menu.options.clear();
                for (int i = 0; i < MAX_RECORDS; ++i)
                {
                    auto record = memory::save_data.records_timeattack[i];
                    auto name = bn::string<RECORD_NAME_LENGTH>(record.name.data(), RECORD_NAME_LENGTH);
                    auto str = bn::format<menu_option::TEXT_MAX_LENGTH>("{} {}", name, record.score);
                    BN_LOG(str);
                    records_timeattack_menu.options.push_back(menu_option(str, menu_option_key::noop));
                }
                change_menu(&records_timeattack_menu);
            }*/

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
                gc.reset();
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
                bool game_done = gc.update();

                if (game_done)
                {
                    auto is_record = memory::is_record(gc.get_mode(), gc.get_level(), gc.get_gamemode_metric()).has_value();

                    if (is_record)
                    {
                        // TODO: Display user's score.
                        change_state(game_state::hiscore);
                    }
                    else
                    {
                        change_state(game_state::menus);
                        // TODO: Display user's gamemode & score to them
                        change_menu(&gameover_menu);

                        while (gameover_menu.options.size() > 2)
                        {
                            gameover_menu.options.pop_back();
                        }

                        text_generator.generate(0, -30, "SCORE", menu_options_sprites[2]);
                        auto score_string = gc.get_gamemode_metric_display_string();
                        text_generator.generate(0, -15, score_string, menu_options_sprites[3]);
                    }
                }
            }
        }
        else if (state == game_state::menus || state == game_state::paused)
        {
            update_menus();
        }
        else if (state == game_state::hiscore)
        {
            auto name_entered = hec.update();
            
            if (name_entered)
            {
                auto name = hec.build_name_array();
                auto mode = gc.get_mode();
                auto level = gc.get_level();
                auto metric = gc.get_gamemode_metric();

                memory::save_record(mode, level, name, metric);
                change_state(game_state::menus);
            }
        }
        else
        {
            BN_ASSERT(false, "Invalid game state");
        }

        bn::core::update();
    }

    main_controller()
    {
        gc.hide();
        hec.hide();
        text_generator.set_center_alignment();

        // Init vectors inside menu_options_sprites.
        for (int i = 0; i < MAX_OPTIONS_PER_SCREEN; ++i)
        {
            menu_options_sprites.push_back(bn::vector<bn::sprite_ptr, LONGEST_OPTION_TEXT>());
        }

        main_menu.options.push_back(menu_option("PLAY", menu_option_key::play));
        main_menu.options.push_back(menu_option("RECORDS", menu_option_key::records));
        main_menu.options.push_back(menu_option("SETTINGS", menu_option_key::settings));
        
        play_menu.options.push_back(menu_option("SPRINT", menu_option_key::play_sprint));
        play_menu.options.push_back(menu_option("TIME ATTACK", menu_option_key::play_timeattack));
        play_menu.options.push_back(menu_option("SURVIVAL", menu_option_key::play_survival));

        records_menu.options.push_back(menu_option("SPRINT", menu_option_key::records_sprint));
        records_menu.options.push_back(menu_option("TIME ATTACK", menu_option_key::records_timeattack));

        settings_menu.options.push_back(menu_option(memory::sfx_enabled() ? "SFX ON" : "SFX OFF", menu_option_key::sfx_toggle));
        settings_menu.options.push_back(menu_option(memory::music_enabled() ? "MUSIC ON" : "MUSIC OFF", menu_option_key::music_toggle));

        pause_menu.options.push_back(menu_option("RESUME", menu_option_key::resume));
        pause_menu.options.push_back(menu_option("RESTART", menu_option_key::restart));
        pause_menu.options.push_back(menu_option("QUIT", menu_option_key::quit));

        gameover_menu.options.push_back(menu_option("RETRY", menu_option_key::restart));
        gameover_menu.options.push_back(menu_option("MENU", menu_option_key::quit));
        gameover_menu.options_count = 2;
        gameover_menu.options_y_position = 30;

        change_menu(&main_menu);
    }
};