#pragma once

#include "bn_bg_palettes.h"
#include "bn_core.h"
#include "bn_fixed_point.h"
#include "bn_fixed.h"
#include "bn_format.h"
#include "bn_keypad.h"
#include "bn_list.h"
#include "bn_log.h"
#include "bn_math.h"
#include "bn_music_item.h"
#include "bn_music_items.h"
#include "bn_music.h"
#include "bn_random.h"
#include "bn_seed_random.h"
#include "bn_sound_items.h"
#include "bn_sprite_actions.h"
#include "bn_sprite_items_gem.h"
#include "bn_sprite_items_selector.h"
#include "bn_sprite_items_selector_dir.h"
#include "bn_sprite_items_one.h"
#include "bn_sprite_items_two.h"
#include "bn_sprite_items_three.h"
#include "bn_sprite_palette_item.h"
#include "bn_sprite_palette_ptr.h"
#include "bn_sprite_palettes.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_text_generator.h"
#include "bn_sstream.h"
#include "bn_string.h"
#include "bn_time.h"
#include "bn_unique_ptr.h"
#include "bn_unordered_set.h"
#include "bn_vector.h"
#include "board_drawer.h"
#include "board.h"
#include "gem_type.h"
#include "gj_big_sprite_font.h"
#include "bn_optional.h"
#include "bn_sprite_affine_mat_ptr.h"
#include <bn_sprite_double_size_mode.h>
#include "bn_music_items.h"
#include "floating_text.h"
#include "util.h"
#include "game_mode.h"
#include "levels.h"
#include "music_util.h"
#include "bn_affine_bg_items_board.h"
#include "bn_affine_bg_ptr.h"
#include "background_controller.h"
#include "board_anims.h"

#define HEADER_X +70    // x position for headers, with left-align generation.
#define VALUE_X +116    // x position for values, with right-align generation.
#define MATCH_SOUNDS_LEN 5  // The amount of unique match sounds.

// Possible states of the game while `update()` is being called.
enum class game_state
{
    countdown,  // Short 3-2-1 visual countdown before swaps are allowed to be made.
    playing,    // The actual gameplay state, user can make all inputs freely.
    gameover,   // Small period of animation before the gameover/hiscore screen appears.
};

enum class gameover_state
{
    board_finishing_animation,  // Input disbled, wait for the board drawer animation to stop, then greyout the board.
    greying_out_board,   // 1 second greying out the board gems one by one.
    waiting_for_score_counter,  // Wait for displayed_score to reach the actual score, then greyout all text.
    greying_out_text,  // Period for greying out all text on the right of the game screen.
    small_pause_before_menu, // Short period after the score counter stops, 1 second wait, before showing the gameover/hiscore menu.
};

// Handles user input, passing it to `board`, and managing `board_drawer`'s animations, tracking score and combo.
class game_controller
{
private:
    board b;
    board_drawer bd = board_drawer(b);
    background_controller& background;
    bn::affine_bg_ptr board_bg = bn::affine_bg_items::board.create_bg(11, 51);
    bn::sprite_text_generator text_generator = bn::sprite_text_generator(gj::fixed_32x64_sprite_font);
    bn::vector<bn::sprite_ptr, 5> header_score;  // Header text sprites that say "SCORE".
    bn::vector<bn::sprite_ptr, 5> header_combo;    // Header text sprites that say "COMBO".
    bn::vector<bn::sprite_ptr, 4> header_time;    // Header text sprites that say "TIME".
    bn::vector<bn::sprite_ptr, 5> header_goal_or_limit;    // Header text sprites that say either "GOAL" or "LIMIT".
    bn::vector<bn::sprite_ptr, 32> value_score;
    bn::vector<bn::sprite_ptr, 4> value_combo;
    bn::vector<bn::sprite_ptr, 5> value_time;
    bn::vector<bn::sprite_ptr, 5> value_goal_or_limit;   // Text sprites for the goal/limit value.
    bn::sprite_ptr spr_selector = bn::sprite_items::selector.create_sprite(0, 0);
    bn::sprite_ptr spr_selector_dirs = bn::sprite_items::selector_dir.create_sprite(0, 0);  // Selector sprite with direction arrows.
    bn::sprite_palette_ptr text_palette = gj::fixed_32x64_sprite_font.item().palette_item().create_palette();
    bn::sprite_palette_ptr selector_active_palette = spr_selector.palette();
    bn::sprite_palette_ptr selector_inactive_palette = create_palette(16, 16, 16);
    bn::sprite_palette_ptr gameover_grey = create_palette(3, 3, 3);
    bn::sprite_palette_ptr gameover_selector = create_palette(5, 5, 5);
    bn::sprite_palette_ptr gameover_grey_text = create_text_palette(bn::color(4, 4, 4), bn::color(1, 1, 1));
    bn::optional<bn::sprite_ptr> countdown_number_sprite;
    bn::vector<floating_text, 64> floating_texts;

    game_state state = game_state::countdown;
    int sel_row = 0;    // Current row of the selector.
    int sel_col = 0;    // Current column of the selector.
    int displayed_score = 0;
    int start_countdown_timer_frames = 0;    // How many frames remain until the game starts.
    bool board_animating = false;   // Is the board currently animating.

    // Gameover animation state, prefixed with `go_`.
    const int go_frames_per_text_element = 4;
    gameover_state go_state;    // Current stage of the gameover animation.
    BOARD_ANIM go_board_greyout_anim;   // Animation order for the board greyout.
    int go_frames_greying_out_board = 0;    // How many frames spent waiting for board to grey out.
    int go_frames_greying_out_text = 0; // How many frames spent waiting for text elements to grey out.
    int go_frames_menu_pause = 0; // How many frames spent waiting for menu to appear after score finished counting.

    // Sound items for matches, increasing in pitch, use higher numbers for combos.
    bn::sound_item match_sounds[MATCH_SOUNDS_LEN] = {
        bn::sound_items::match1,
        bn::sound_items::match2,
        bn::sound_items::match3,
        bn::sound_items::match4,
        bn::sound_items::match5,
    };
    
    // Game mode variables.
    game_mode mode;
    int level;
    int combo = 1;
    int score = 0;
    int timer_frames = 0;   // Amount of update frames since last reset.
    int score_goal = 0; // For sprint game mode, what is the player's goal score.
    int time_limit_frames = -1; // For time-attack game mode, what is the player's time limit (in frames).

    void animate_floating_texts()
    {
        for(auto it = floating_texts.begin(), end = floating_texts.end(); it != end;)
        {
            if(it->update())
            {
                floating_texts.erase(it);
                end = floating_texts.end();
            }
            else
            {
                ++it;
            }
        }
    }

    // Grey out all the ui elements.
    void greyout_board()
    {
        //spr_selector.set_palette(gameover_grey);      // Now invisible.
        //spr_selector_dirs.set_palette(gameover_grey);
        background.brake();
        //bd.greyout(); // Doing an animation for this now.
    }

    void greyout_text()
    {
        for (auto& s : header_score) { s.set_palette(gameover_grey_text); }
        for (auto& s : header_combo) { s.set_palette(gameover_grey_text); }
        for (auto& s : header_time) { s.set_palette(gameover_grey_text); }
        for (auto& s : header_goal_or_limit) { s.set_palette(gameover_grey_text); }
        for (auto& s : value_goal_or_limit) { s.set_palette(gameover_grey_text); }
        for (auto& s : value_time) { s.set_palette(gameover_grey_text); }
        for (auto& s : value_score) { s.set_palette(gameover_grey_text); }
        for (auto& s : value_combo) { s.set_palette(gameover_grey_text); }
        for (auto& ft : floating_texts) { ft.set_palette(gameover_grey_text); }
    }

    void draw_score()
    {
        int jiggle = (displayed_score < score && displayed_score % 2 == 0 ? 1 : 0);    // Jiggle text up and down each frame while displayed score is incrementing.

        value_score.clear();
        text_generator.generate(VALUE_X, -62 - jiggle, bn::to_string<32>(displayed_score), value_score);   // TODO: Fix Y position to align with gems border when added.
    }

    // Heart and soul of the game. Updates the board frame by frame to perform animations, once the animations complete
    // depending on which animations they were, the next piece of logic takes place. Fragile code, be careful.
    void update_board()
    {
        auto board_animations_complete = bd.update();
        if (board_animations_complete)
        {
            // When the drawer says the animations are complete, the state variable is whatever it
            // was last animation, by knowing what it was was last animation we can tell what animations just completed.
            // Then we can decide what to do next.
            auto bdstate = bd.state();

            // When a slide or gem drops complete, delete matches again.
            if (bdstate == drawer_state::PlayingSlide || bdstate == drawer_state::DroppingGems)
            {
                auto matches = b.delete_matches();

                /*if (!matches.empty())
                {
                    BN_LOG(
                        "SOME MATCHES WERE FOUND.",
                        "board_animations_complete: ", board_animations_complete, 
                        "bdstate: ", (int)bdstate,
                        "matches count: ", matches.size()
                    );
                }*/

                for (auto& m : matches)
                {
                    // TODO: Score adjustments:
                    // - Should each gem be worth worth its match size? E.g. A 4-of-a-kind each gem gives 4 points?
                    // - Should each match increase the combo rather than each play_matches?
                    auto points_per_gem = m.positions.size() * combo;
                    auto points_for_match = points_per_gem * m.positions.size();
                    score += points_for_match;
                    
                    // TODO: Small innacuracy in displayed floating scores when 1 gem is used in 2 matches, the user
                    // only sees the top-most point for a single match.
                    for (auto& p : m.positions)
                    {
                        auto palette = bd.palettes[(int)m.type];
                        auto ft = floating_text(positions[p.row][p.col], palette, points_per_gem);
                        floating_texts.push_back(ft);
                    }

                    background.accelerate();
                }

                if (!matches.empty())
                {
                    auto& sound = match_sounds[bn::min(combo - 1, MATCH_SOUNDS_LEN - 1)];
                    sound.play();
                }

                bd.play_matches(matches);
                combo += 1;
            }
            // After matches are destroyed, drop gems.
            else if (bdstate == drawer_state::DestroyingMatches)
            {
                auto drops = b.drop_gems();
                bd.play_drops(drops);
            }
            else // State switches to `Waiting` when play_matches() is called with an empty list.
            {
                board_animating = false;
                combo = 1;
            }
        }
        else
        {
            board_animating = true;
        }
    }

    void update_displayed_score()
    {
        if (displayed_score < score)
        {
            if (displayed_score % 2 == 0)
            {
                bn::sound_items::point.play();
            }
            
            ++displayed_score;
        }

        // OPTIMISATION: Can optimise by only re-generating sprites if the displayed_score is different from last frame.
        // 6 characters can fit before overflowing onto the game board.
        draw_score();
    }

    void update_countdown()
    {
        if (start_countdown_timer_frames > 0)
        {
            int next_threshold;
            if (start_countdown_timer_frames > 120)
            {
                countdown_number_sprite = bn::sprite_items::three.create_sprite(0, 0);
                next_threshold = 120;
            }
            else if (start_countdown_timer_frames > 60)
            {
                countdown_number_sprite = bn::sprite_items::two.create_sprite(0, 0);
                next_threshold = 60;
            }
            else
            {
                countdown_number_sprite = bn::sprite_items::one.create_sprite(0, 0);
                next_threshold = 0;
            }

            //countdown_number_sprite.value().set_double_size_mode(bn::sprite_double_size_mode::ENABLED);
            auto scale = bn::fixed(1.5);
            scale -= 0.01 * (next_threshold - start_countdown_timer_frames);
            countdown_number_sprite.value().set_scale(scale);

            if (start_countdown_timer_frames == 180 || start_countdown_timer_frames == 120 || start_countdown_timer_frames == 60)
            {
                bn::sound_items::countdown_beep.play();
            }
            else if (start_countdown_timer_frames == 1)
            {
                bn::sound_items::countdown_finish_beep.play();
                music_util::play_random();
            }

            --start_countdown_timer_frames;
        }
        else
        {
            countdown_number_sprite.reset();
            state = game_state::playing;
        }
    }

    // Update routine for the gameover state.
    bool update_gameover()
    {
        if (go_state == gameover_state::board_finishing_animation)
        {
            update_board();
            update_displayed_score();

            if (!board_animating)
            {
                greyout_board();
                go_state = gameover_state::greying_out_board;
            }
        }
        else if (go_state == gameover_state::greying_out_board)
        {
            // This step waits 29 frames, greying out the board 1 by 1 until fully greyedout.
            update_displayed_score();

            // 5 rows * 6 cols = 30 gems, this frame counter ranges 0 - 29
            auto number = go_frames_greying_out_board++;

            // Search through the animation to find the next gem to grey out.
            int row = -1, col = -1;   // Set these to -1, so once they are set the loops break.
            for (int r = 0; r < board::rows && row == -1; ++r)
            {
                for (int c = 0; c < board::cols && col == -1; ++c)
                {
                    if (go_board_greyout_anim[r][c] == number)
                    {
                        row = r;    // Setting these will break the loops.
                        col = c;
                    }
                }
            }

            bd.greyout(row, col);

            if (go_frames_greying_out_board == 30)  // After the 29th increment, 30 frames, move to next animation stage.
            {
                go_state = gameover_state::waiting_for_score_counter;
            }
        }
        else if (go_state == gameover_state::waiting_for_score_counter)
        {
            update_displayed_score();

            if (displayed_score == score)
            {
                go_state = gameover_state::greying_out_text;
            }
        }
        else if (go_state == gameover_state::greying_out_text)
        {
            // 7*8 = 56
            // 7 -> 14 -> 21 -> 28 -> 35 -> 42 -> 49 -> 56 -> 63
            //const int text_elements = 8;    // header + value for score, combo, time, limit/goal.

            if (go_frames_greying_out_text >= go_frames_per_text_element * 8)
                go_state = gameover_state::small_pause_before_menu;
            else if (go_frames_greying_out_text > go_frames_per_text_element * 7)
                for (auto s : header_score) s.set_palette(gameover_grey_text);
            else if (go_frames_greying_out_text > go_frames_per_text_element * 6)
                for (auto s : value_score) s.set_palette(gameover_grey_text);
            else if (go_frames_greying_out_text > go_frames_per_text_element * 5)
                for (auto s : header_combo) s.set_palette(gameover_grey_text);
            else if (go_frames_greying_out_text > go_frames_per_text_element * 4)
                for (auto s : value_combo) s.set_palette(gameover_grey_text);
            else if (go_frames_greying_out_text > go_frames_per_text_element * 3)
                for (auto s : header_time) s.set_palette(gameover_grey_text);
            else if (go_frames_greying_out_text > go_frames_per_text_element * 2)
                for (auto s : value_time) s.set_palette(gameover_grey_text);
            else if (go_frames_greying_out_text > go_frames_per_text_element * 1)
                for (auto s : header_goal_or_limit) s.set_palette(gameover_grey_text);
            else if (go_frames_greying_out_text > 0)
                for (auto s : value_goal_or_limit) s.set_palette(gameover_grey_text);

            ++go_frames_greying_out_text;
        }
        else if (go_state == gameover_state::small_pause_before_menu)
        {
            // Tiny pause matching the text element greyouts.
            if (++go_frames_menu_pause >= go_frames_per_text_element)
            {
                return true;
            }
        }
        else
        {
            BN_ASSERT(false, "Unknown go_state: ", (int)go_state);
        }

        // Call this repeatedly while doing the gameover animation, it will continue to 
        // be called in main_controller on the gameover/hiscore screens.
        music_util::slowdown();
        animate_floating_texts();
        return false;
    }

    // Start the gameover animation process.
    // Checked for at the start of each `update()`.
    void start_gameover()
    {
        spr_selector.set_visible(false);
        spr_selector_dirs.set_visible(false);
        state = game_state::gameover;
        go_board_greyout_anim = board_anims::get_random();
        go_state = gameover_state::board_finishing_animation;
        go_frames_greying_out_board = 0;
        go_frames_greying_out_text = 0;
        go_frames_menu_pause = 0;
    }
public: 
    // Constructor.
    game_controller(background_controller& _background) : background(_background)
    {
        text_generator.set_left_alignment();
        text_generator.generate(HEADER_X, -71, "SCORE", header_score);   // TODO: Fix Y position to align with gems border when added.
        text_generator.generate(HEADER_X, -50, "COMBO", header_combo);
        text_generator.generate(HEADER_X, +40, "TIME", header_time);
        // LIMIT/GOAL text is generated in newgame function.
        text_generator.set_right_alignment();

        text_generator_small.set_center_alignment();

        spr_selector_dirs.set_visible(false);
        board_bg.set_wrapping_enabled(false);
    }

    game_mode get_mode() { return mode; }
    int get_level() { return level; }

    // Get the metric relevant to the current gamemode (either score or frametime).
    int get_gamemode_metric()
    {
        if (mode == game_mode::sprint) { return timer_frames; }
        else if (mode == game_mode::timeattack) { return score; }
        else { BN_ASSERT(false, "Unknown game mode: ", (int)mode); return 0; }
    }

    bn::string<8> get_gamemode_metric_display_string()
    {
        auto metric = get_gamemode_metric();
        if (mode == game_mode::sprint) { return util::frames_to_time_millis_string(metric); }
        else if (mode == game_mode::timeattack) { return bn::to_string<8>(metric); }
        else { BN_ASSERT(false, "Unknown game mode: ", (int)mode); return bn::to_string<1>(0); }
    }

    bool can_pause()
    {
        // Don't allow pause during the gameover state because restoring the 
        // gameover animation upon resume is too difficult at this point.
        return state != game_state::gameover;
    }

    // Greyout/freeze the game to be displayed in the background of the pause menu.
    void pause()
    {
        for (auto& s : header_score) { s.set_palette(gameover_grey_text); }
        for (auto& s : header_combo) { s.set_palette(gameover_grey_text); }
        for (auto& s : header_time) { s.set_palette(gameover_grey_text); }
        for (auto& s : header_goal_or_limit) { s.set_palette(gameover_grey_text); }
        for (auto& s : value_goal_or_limit) { s.set_palette(gameover_grey_text); }
        for (auto& s : value_time) { s.set_palette(gameover_grey_text); }
        for (auto& ft : floating_texts) { ft.set_palette(gameover_grey_text); }
        for (auto& s : value_score) { s.set_palette(gameover_grey_text); }
        for (auto& s : value_combo) { s.set_palette(gameover_grey_text); }
        if (countdown_number_sprite.has_value()) { countdown_number_sprite->set_visible(false); }
        spr_selector.set_palette(gameover_selector);
        spr_selector_dirs.set_palette(gameover_selector);
        bd.greyout();
        background.freeze();
    }

    void hide()
    {
        for (auto& s : header_score) { s.set_visible(false); }
        for (auto& s : header_combo) { s.set_visible(false); }
        for (auto& s : header_time) { s.set_visible(false); }
        for (auto& s : header_goal_or_limit) { s.set_visible(false); }
        for (auto& s : value_goal_or_limit) { s.set_visible(false); }
        for (auto& s : value_time) { s.set_visible(false); }
        for (auto& ft : floating_texts) { ft.set_visible(false); }
        if (countdown_number_sprite.has_value()) { countdown_number_sprite.value().set_visible(false); }
        
        board_bg.set_visible(false);
        value_score.clear();
        value_combo.clear();
        spr_selector.set_visible(false);
        spr_selector_dirs.set_visible(false);
        bd.hide();
    }

    void show()
    {
        for (auto& s : header_score) { s.set_visible(true); s.set_palette(text_palette); }
        for (auto& s : header_combo) { s.set_visible(true); s.set_palette(text_palette); }
        for (auto& s : header_time) { s.set_visible(true); s.set_palette(text_palette); }
        for (auto& s : header_goal_or_limit) { s.set_visible(true); s.set_palette(text_palette); }
        for (auto& s : value_goal_or_limit) { s.set_visible(true); s.set_palette(text_palette); }
        for (auto& ft : floating_texts) { ft.set_visible(true); ft.set_palette(text_palette); }

        board_bg.set_visible(true);
        spr_selector.set_visible(true);
        spr_selector_dirs.set_visible(false);
        bd.show();
        background.thaw();
    }

    void reset(unsigned int frames_since_boot)
    {
        // Reset game tracking state.
        state = game_state::countdown;
        score = 0;
        displayed_score = 0;
        combo = 1;
        board_animating = false;
        start_countdown_timer_frames = 60 * 3;
        timer_frames = 0;

        // Set random seed & create new board.
        b.set_seed(frames_since_boot);
        b.new_board();

        // Reset GUI.
        bd.reset();
        bd.animate_random_drop_all_in();
        floating_texts.clear();

        music_util::stop();

        background.reset();
    }

    void newgame(game_mode _mode, int _level, unsigned int frames_since_boot)
    {
        reset(frames_since_boot);
        this->mode = _mode;
        this->level = _level;
        this->score_goal = mode == game_mode::sprint ? levels::sprint[level] : 0;
        this->time_limit_frames = mode == game_mode::timeattack ? levels::timeattack[level] : 0;

        header_goal_or_limit.clear();
        text_generator.set_left_alignment();
        text_generator.generate(HEADER_X, +61, mode == game_mode::sprint ? "GOAL" : "LIMIT", header_goal_or_limit);
        text_generator.set_right_alignment();

        value_goal_or_limit.clear();
        auto goal_limit_value = mode == game_mode::sprint ? bn::to_string<5>(score_goal) : util::frames_to_time_string(time_limit_frames);
        text_generator.generate(VALUE_X, +70, goal_limit_value, value_goal_or_limit);
    }

    // Returns true if game is complete, based on different conditions depending on game mode.
    bool update()
    {
        if (state == game_state::gameover)
        {
            return update_gameover();
        }

        if (state == game_state::countdown)
        {
            // We don't return from this state to let the board drop-in animation to play
            // and allow the player to move / interact with the selector.
            update_countdown();
        }

        auto can_swap = !board_animating && state == game_state::playing; // Is the player allowed to swap a gem this frame

        // If A is held then prevent selector movement.
        if (bn::keypad::a_held())
        {
            int move_row = 0;
            int move_col = 0;

            // TODO: Validation with error sfx and animation.
            if (bn::keypad::left_pressed() && sel_col > 0)
            {
                move_col = -1;
            }
            else if (bn::keypad::right_pressed() && sel_col < board::cols - 1)
            {
                move_col = +1;
            }
            else if (bn::keypad::up_pressed() && sel_row > 0)
            {
                move_row = -1;
            }
            else if (bn::keypad::down_pressed() && sel_row < board::rows - 1)
            {
                move_row = +1;
            }

            // Only execute swap if the board isn't currently animating.
            if (can_swap && (move_row != 0 || move_col != 0))
            {
                b.swap(sel_row, sel_col, sel_row + move_row, sel_col + move_col);
                bd.slide(sel_row, sel_col, sel_row + move_row, sel_col + move_col);
                bn::sound_items::slide.play();
            }
        }
        else
        {
            if (bn::keypad::left_pressed() && sel_col > 0)
            {
                sel_col -= 1;
                bn::sound_items::move_selector.play();
            }
            else if (bn::keypad::right_pressed() && sel_col < board::cols - 1)
            {
                sel_col += 1;
                bn::sound_items::move_selector.play();
            }

            if (bn::keypad::up_pressed() && sel_row > 0)
            {
                sel_row -= 1;
                bn::sound_items::move_selector.play();
            }
            else if (bn::keypad::down_pressed() && sel_row < board::rows - 1)
            {
                sel_row += 1;
                bn::sound_items::move_selector.play();
            }
            else if (bn::keypad::select_pressed())
            {
                bd.animate_random_drop_all_in();
            }
        }

        auto& spr_current_selector = bn::keypad::a_held() ? spr_selector_dirs : spr_selector;
        auto& spr_other_selector = bn::keypad::a_held() ? spr_selector : spr_selector_dirs;
        auto selector_point = positions[sel_row][sel_col];
        spr_current_selector.set_position(selector_point);
        spr_current_selector.set_palette(can_swap ? selector_active_palette : selector_inactive_palette);
        spr_current_selector.set_visible(true);
        spr_other_selector.set_visible(false);

        update_board();

        update_displayed_score();

        value_combo.clear();
        text_generator.generate(VALUE_X, -41, bn::format<4>("x{}", combo), value_combo);
        
        auto time_str = util::frames_to_time_string(timer_frames);
        value_time.clear();
        text_generator.generate(VALUE_X, +49, time_str, value_time);

        if (state == game_state::playing)
        {
            ++timer_frames;
        }

        animate_floating_texts();

        // Check for game-end conditions.
        auto sprint_gameover = mode == game_mode::sprint && score >= score_goal;
        auto timeattack_gameover = mode == game_mode::timeattack && timer_frames > time_limit_frames;
        if (sprint_gameover || timeattack_gameover)
        {
            start_gameover();
        }

        return false;
    }
};

