#pragma once

#include "game_controller.h"
#include "menu.h"
#include "menu_option.h"
#include "overall_state.h"
#include "menu_option_key.h"
#include "memory.h"
#include "bn_string.h"
#include "highscore_entry_controller.h"
#include "game_mode.h"
#include "bn_sound.h"
#include "bn_music.h"
#include "sound_util.h"
#include "music_util.h"
#include "background_controller.h"

#define TITLE_Y -55

enum class menu_sound
{
    ok,
    back,
    none,
};

class main_controller
{
private:
    game_controller gc;
    highscore_entry_controller hec;
    background_controller& background;
    overall_state state = overall_state::menus;
    int selected_index = 0;
    bn::sprite_text_generator text_generator = bn::sprite_text_generator(gj::fixed_32x64_sprite_font);
    bn::sprite_palette_ptr palette_highlight = gj::fixed_32x64_sprite_font.item().palette_item().create_palette();
    bn::sprite_palette_ptr palette_grey = create_text_palette(bn::color(5, 5, 5), bn::color(0, 0, 0));
    bn::vector<bn::sprite_ptr, LONGEST_TITLE_TEXT> menu_title_sprites;
    bn::fixed title_sin_angle;
    bn::fixed title_sin_angle_inc = 5;
    bn::vector<bn::vector<bn::sprite_ptr, LONGEST_OPTION_TEXT>, MAX_OPTIONS_PER_SCREEN> menu_options_sprites;

    menu main_menu = menu("GEMATRIX");
    menu play_menu = menu("PLAY", &main_menu);  // Game start mode selection.
    menu play_levels_menu = menu("LEVEL", &play_menu);  // After the user selects a mode on the `play_menu`, they select the level on this one.
    menu records_menu = menu("RECORDS", &main_menu);    // Record viewing game mode selection.
    menu records_levels_menu = menu("RECORDS LEVEL", &records_menu);    // Menu used for record level select, is altered to be used for different modes.
    menu records_display_menu = menu("X RECORDS", &records_levels_menu);    // Menu used for record list display, is altered to be used for different modes/levels.
    menu settings_menu = menu("SETTINGS", &main_menu);  // Options: [0] = SFX, [1] = MUSIC.
    menu pause_menu = menu("PAUSE");    // Pause menu used in game.
    menu gameover_menu = menu("GAMEOVER");  // Gameover menu used when user's score was not a record. Hiscore entry is its own state.
    menu* current_menu = &main_menu;

    // The user's desired game mode when on the `play_levels_menu` or `records_levels_menu`.
    game_mode levels_mode;
    bn::string_view sprint_levels_menu_title = "GOAL SCORE";
    bn::string_view timeattack_levels_menu_title = "TIME LIMIT";
    bn::string_view sprint_records_levels_menu_title = "SPRINT RECORDS";
    bn::string_view timeattack_records_levels_menu_title = "TIMEATTACK RECORDS";
    bn::string<LONGEST_TITLE_TEXT> records_display_menu_title_text = "";    // Title text for the records display menu. As `menu` uses a string view for the title this has to be stored somewhere constant.

    void set_title_text(const bn::string_view& text)
    {
        text_generator.set_one_sprite_per_character(true);  // Generate one sprite per char so we can sin-wave them.
        text_generator.generate(0, TITLE_Y, text, menu_title_sprites);
        text_generator.set_one_sprite_per_character(false); // Set it back so other stuff doesn't have to worry about it.
    }

    void generate_options_text()
    {
        for (int i = 0; i < current_menu->options.max_size(); ++i)
        {
            menu_options_sprites[i].clear();
        }

        auto size = current_menu->get_options_count();
        auto y_pos = current_menu->get_y_position();
        for (int i = 0; i < size; ++i)
        {
            text_generator.generate(0, y_pos + (i * 20), current_menu->options[i].text, menu_options_sprites[i]);

            // Annoyingly set the palette of each sprite here so they don't all flicker as white for one frame.
            if (current_menu != &records_display_menu)  // Except when on records_display_menu so all records display in normal highlighted palette.
                for (auto s : menu_options_sprites[i])
                    s.set_palette(i == selected_index ? palette_highlight : palette_grey);
        }
    }

    // Set to `nullptr` to hide menu.
    void change_menu(menu* new_menu, menu_sound sound)
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
            set_title_text(current_menu->title);

            // Options
            generate_options_text();
        }

        if (sound == menu_sound::ok) { bn::sound_items::menu_ok.play(); }
        else if (sound == menu_sound::back) { bn::sound_items::menu_back.play(); }
    }

    /// @brief Change the state of the overall game, making required adjustments to display and input.
    /// @param new_state The new state to change to.
    /// @param hide_game Should the game be hidden. Most often yes, but in some cases (e.g. gameover/hiscore entry) we leave the game greyed out in the background.
    void change_state(overall_state new_state, bool hide_game = true)
    {
        state = new_state;

        if (state == overall_state::menus)
        {
            change_menu(&main_menu, menu_sound::none);
            hec.hide();
        }
        else if (state == overall_state::paused)
        {
            gc.pause();
            music_util::maybe_pause();
            bn::sound_items::pause.play();
            change_menu(&pause_menu, menu_sound::none);
        }
        else if (state == overall_state::ingame)
        {
            music_util::maybe_resume();
            change_menu(nullptr, menu_sound::ok);
            gc.show();
        }
        else if (state == overall_state::hiscore)
        {
            change_menu(nullptr, menu_sound::none);

            auto record_position = memory::is_record(gc.get_mode(), gc.get_level(), gc.get_gamemode_metric());
            set_title_text("HISCORE!");
            
            if (record_position.has_value())
            {
                auto mode_name = gc.get_mode() == game_mode::sprint ? "SPRINT" : "TIMEATTACK";
                auto ordinal_position = util::ordinal_string(record_position.value());  // Ordinal position text (e.g. 1st, 3rd, etc).
                auto mode_position_text = bn::format<menu_option::TEXT_MAX_LENGTH>("{} {}", mode_name, ordinal_position); // E.g: "TIMEATTACK 1st"
                text_generator.generate(0, -30, mode_position_text, menu_options_sprites[0]);
                
                auto score_str = gc.get_gamemode_metric_display_string();
                text_generator.generate(0, -15, score_str, menu_options_sprites[1]);

                text_generator.generate(0, 30, "ENTER NAME:", menu_options_sprites[2]);
            }
        }

        if (state != overall_state::ingame && state != overall_state::paused && hide_game)
        {
            gc.hide();
        }
    }

    // Technique stolen from examples/text/main.cpp:sprite_per_character_text_scene().
    void sin_wave_title()
    {
        title_sin_angle += title_sin_angle_inc;

        if(title_sin_angle >= 360)
        {
            title_sin_angle -= 360;
        }

        bn::fixed local_angle = title_sin_angle;

        for(auto& character_sprite : menu_title_sprites)
        {
            local_angle += title_sin_angle_inc;

            if (local_angle >= 360)
            {
                local_angle -= 360;
            }

            character_sprite.set_y(TITLE_Y + bn::degrees_lut_sin(local_angle) * 4);
        }
    }

    void update_menus()
    {
        sin_wave_title();

        // Records display menu is not interactable.
        auto menu_interactable = current_menu != &records_display_menu;

        if (bn::keypad::b_pressed())
        {
            if (current_menu->previous_menu != nullptr)
            {
                change_menu(current_menu->previous_menu, menu_sound::back);
            }
        }

        if (!menu_interactable)
        {
            return;
        }

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
        else if (bn::keypad::start_pressed() && state == overall_state::paused)    // Start button shortcut for resume when paused.
        {
            change_state(overall_state::ingame);
        }
        else if (bn::keypad::a_pressed())
        {
            auto key = current_menu->options[selected_index].key;
            
            // TODO: Pull this long if-else chain out into its own function/switch block.
            
            // MENU: MAIN
            if (key == menu_option_key::play)
            {
                change_menu(&play_menu, menu_sound::ok);
            }
            else if (key == menu_option_key::records)
            {
                change_menu(&records_menu, menu_sound::ok);
            }
            else if (key == menu_option_key::settings)
            {
                change_menu(&settings_menu, menu_sound::ok);
            }

            // MENU: PLAY
            else if (key == menu_option_key::play_sprint)
            {
                levels_mode = game_mode::sprint;

                play_levels_menu.title = sprint_levels_menu_title;
                play_levels_menu.options.clear();
                play_levels_menu.options.push_back(menu_option(bn::to_string<5>(levels::sprint[0]), menu_option_key::play_level0));
                play_levels_menu.options.push_back(menu_option(bn::to_string<5>(levels::sprint[1]), menu_option_key::play_level1));
                play_levels_menu.options.push_back(menu_option(bn::to_string<5>(levels::sprint[2]), menu_option_key::play_level2));

                change_menu(&play_levels_menu, menu_sound::ok);
            }
            else if (key == menu_option_key::play_timeattack)
            {
                levels_mode = game_mode::timeattack;

                play_levels_menu.title = timeattack_levels_menu_title;
                play_levels_menu.options.clear();
                play_levels_menu.options.push_back(menu_option(util::frames_to_time_string(levels::timeattack[0]), menu_option_key::play_level0));
                play_levels_menu.options.push_back(menu_option(util::frames_to_time_string(levels::timeattack[1]), menu_option_key::play_level1));
                play_levels_menu.options.push_back(menu_option(util::frames_to_time_string(levels::timeattack[2]), menu_option_key::play_level2));

                change_menu(&play_levels_menu, menu_sound::ok);
            }

            // MENU: PLAY LEVEL SELECT
            else if (key >= menu_option_key::play_level0 && key <= menu_option_key::play_level2)
            {
                auto level = (int)key - (int)menu_option_key::play_level0;
                gc.newgame(levels_mode, level);
                change_state(overall_state::ingame);
            }

            // MENU: RECORDS
            else if (key == menu_option_key::records_sprint || key == menu_option_key::records_timeattack)
            {
                auto is_sprint = key == menu_option_key::records_sprint;
                levels_mode = is_sprint ? game_mode::sprint : game_mode::timeattack;

                records_levels_menu.title = is_sprint ? sprint_records_levels_menu_title : timeattack_records_levels_menu_title;
                records_levels_menu.options.clear();

                for (int i = 0; i < LEVELS; ++i)
                {
                    auto text = is_sprint ? bn::to_string<5>(levels::sprint[i]) : util::frames_to_time_string(levels::timeattack[i]);
                    auto ke = (menu_option_key)((int)menu_option_key::records_level0 + i);
                    records_levels_menu.options.push_back(menu_option(text, ke));
                }

                change_menu(&records_levels_menu, menu_sound::ok);
            }

            // MENU: RECORDS LEVEL SELECT
            else if (key >= menu_option_key::records_level0 && key <= menu_option_key::records_level2)
            {
                auto level = (int)key - (int)menu_option_key::records_level0;

                auto mode_text = levels_mode == game_mode::sprint ? "SPRINT" : "TIMEATTACK";
                auto level_text = levels_mode == game_mode::sprint ? bn::to_string<5>(levels::sprint[level]) : util::frames_to_time_string(levels::timeattack[level]);
                records_display_menu_title_text = bn::format<18>("{} ({})", mode_text, level_text); // Longest string is "TIMEATTACK (00:00)"
                records_display_menu.title = records_display_menu_title_text;

                records_display_menu.options.clear();
                for (int i = 0; i < MAX_RECORDS; ++i)
                {
                    auto is_sprint = levels_mode == game_mode::sprint;

                    auto name = is_sprint ? memory::save_data.records_sprint[level][i].name : memory::save_data.records_timeattack[level][i].name;
                    auto metric = is_sprint ? memory::save_data.records_sprint[level][i].time_in_frames : memory::save_data.records_timeattack[level][i].score;

                    auto name_str = bn::string<RECORD_NAME_LENGTH>(name.data(), RECORD_NAME_LENGTH);
                    auto metric_text = is_sprint ? util::frames_to_time_millis_string(metric) : bn::to_string<8>(metric);
                    auto str = bn::format<menu_option::TEXT_MAX_LENGTH>("{} {}", name_str, metric_text);
                    BN_LOG(str);
                    records_display_menu.options.push_back(menu_option(str, menu_option_key::noop));
                }
                change_menu(&records_display_menu, menu_sound::ok);
            }

            // MENU: SETTINGS
            else if (key == menu_option_key::sfx_toggle)
            {
                auto new_setting = !memory::sfx_enabled();
                sound_util::set_sound_enabled(new_setting);
                memory::sfx_enabled_set(new_setting);
                memory::save();
                settings_menu.options[0].text = new_setting ? "SFX ON" : "SFX OFF";
                generate_options_text();
                bn::sound_items::menu_ok.play();
            }
            else if (key == menu_option_key::music_toggle)
            {
                auto new_setting = !memory::music_enabled();
                if (!new_setting)
                {
                    music_util::maybe_stop();
                }
                memory::music_enabled_set(new_setting);
                memory::save();
                settings_menu.options[1].text = new_setting ? "MUSIC ON" : "MUSIC OFF";
                generate_options_text();
                bn::sound_items::menu_ok.play();
            }

            // MENU: PAUSE
            else if (key == menu_option_key::resume)
            {
                change_state(overall_state::ingame);
            }
            else if (key == menu_option_key::restart)
            {
                music_util::maybe_stop();
                gc.reset();
                change_state(overall_state::ingame);
            }
            else if (key == menu_option_key::quit)
            {
                background.reset();
                change_state(overall_state::menus);
                music_util::maybe_stop();
            }
        }
    }

public:
    void update()
    {
        if (state == overall_state::ingame)
        {
            if (bn::keypad::start_pressed() && gc.can_pause())
            {
                change_state(overall_state::paused);
            }
            else
            {
                bool game_done = gc.update();

                if (game_done)
                {
                    auto is_record = memory::is_record(gc.get_mode(), gc.get_level(), gc.get_gamemode_metric()).has_value();

                    if (is_record)
                    {
                        change_state(overall_state::hiscore, /*hide_game: */ false);
                    }
                    else
                    {
                        change_state(overall_state::menus, /*hide_game: */ false);
                        change_menu(&gameover_menu, menu_sound::none);

                        while (gameover_menu.options.size() > 2)
                        {
                            gameover_menu.options.pop_back();
                        }

                        auto time_or_score_string = gc.get_mode() == game_mode::sprint ? "TIME:" : "SCORE:";
                        text_generator.generate(0, -30, time_or_score_string, menu_options_sprites[2]);

                        auto score_string = gc.get_gamemode_metric_display_string();
                        text_generator.generate(0, -15, score_string, menu_options_sprites[3]);
                    }
                }
            }
        }
        else if (state == overall_state::menus || state == overall_state::paused)
        {
            update_menus();
        }
        else if (state == overall_state::hiscore)
        {
            sin_wave_title();
            auto name_entered = hec.update();
            
            if (name_entered)
            {
                auto name = hec.build_name_array();
                auto mode = gc.get_mode();
                auto level = gc.get_level();
                auto metric = gc.get_gamemode_metric();

                memory::save_record(mode, level, name, metric);
                background.reset();
                change_state(overall_state::menus);
            }
        }
        else
        {
            BN_ASSERT(false, "Invalid game state");
        }

        bn::core::update();
    }

    // Constructor.
    main_controller(background_controller& bgc) : gc(bgc), background(bgc)
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
        //play_menu.options.push_back(menu_option("SURVIVAL", menu_option_key::play_survival));

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

        change_menu(&main_menu, menu_sound::none);
    }
};