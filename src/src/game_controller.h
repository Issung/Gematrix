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
    waiting_for_board_to_stop,  // Input disbled, wait for the board drawer animation to stop, then greyout the board.
    waiting_for_board_to_greyout,   // 1 second greying out the board gems one by one.
    waiting_for_score_counter_to_stop,  // Wait for displayed_score to reach the actual score, then greyout all text.
    waiting_for_menu_to_appear, // Short period after the score counter stops, 1 second wait, before showing the gameover/hiscore menu.
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
    bn::vector<bn::sprite_ptr, 5> score_header_text;  // Text sprites that just say "SCORE".
    bn::vector<bn::sprite_ptr, 5> combo_header_text;    // Text sprites that say "COMBO".
    bn::vector<bn::sprite_ptr, 4> time_header_text;    // Text sprites that say "TIME".
    bn::vector<bn::sprite_ptr, 5> goal_or_limit_header_text;    // Text sprites that say either "GOAL" or "LIMIT".
    bn::vector<bn::sprite_ptr, 32> score_number_sprites;
    bn::vector<bn::sprite_ptr, 4> combo_text_sprites;
    bn::vector<bn::sprite_ptr, 5> timer_sprites;
    bn::vector<bn::sprite_ptr, 5> goal_or_limit_text;   // Text sprites for the goal/limit value.
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

    // Gameover animation state
    gameover_state go_state;    // Current stage of the animation.
    BOARD_ANIM board_anim;
    int frames_waiting_for_board_to_greyout = 0;
    int frames_waiting_for_score_to_finish_counting = 0;    // How many frames spent waiting for score to finish counting.
    int frames_waiting_for_gameover_menu_to_appear = 0; // How many frames spent waiting for menu to appear after score finished counting.

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
        for (auto& s : score_header_text) { s.set_palette(gameover_grey_text); }
        for (auto& s : combo_header_text) { s.set_palette(gameover_grey_text); }
        for (auto& s : time_header_text) { s.set_palette(gameover_grey_text); }
        for (auto& s : goal_or_limit_header_text) { s.set_palette(gameover_grey_text); }
        for (auto& s : goal_or_limit_text) { s.set_palette(gameover_grey_text); }
        for (auto& s : timer_sprites) { s.set_palette(gameover_grey_text); }
        for (auto& s : score_number_sprites) { s.set_palette(gameover_grey_text); }
        for (auto& s : combo_text_sprites) { s.set_palette(gameover_grey_text); }
        for (auto& ft : floating_texts) { ft.set_palette(gameover_grey_text); }
    }

    void draw_score()
    {
        int jiggle = (displayed_score < score && displayed_score % 2 == 0 ? 1 : 0);    // Jiggle text up and down each frame while displayed score is incrementing.

        score_number_sprites.clear();
        text_generator.generate(VALUE_X, -62 - jiggle, bn::to_string<32>(displayed_score), score_number_sprites);   // TODO: Fix Y position to align with gems border when added.
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
                        auto palette = bd.colors[(int)m.type];
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

    // Start the gameover animation process.
    // Checked for at the start of each `update()`.
    void start_gameover()
    {
        spr_selector.set_visible(false);
        spr_selector_dirs.set_visible(false);
        state = game_state::gameover;
        board_anim = board_anims::get_random();
        go_state = gameover_state::waiting_for_board_to_stop;
        frames_waiting_for_board_to_greyout = 0;
        frames_waiting_for_score_to_finish_counting = 0;
        frames_waiting_for_gameover_menu_to_appear = 0;
    }
public: 
    // Constructor.
    game_controller(background_controller& _background) : background(_background)
    {
        text_generator.set_left_alignment();
        text_generator.generate(HEADER_X, -71, "SCORE", score_header_text);   // TODO: Fix Y position to align with gems border when added.
        text_generator.generate(HEADER_X, -50, "COMBO", combo_header_text);
        text_generator.generate(HEADER_X, +40, "TIME", time_header_text);
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

    void hide()
    {
        for (auto& s : score_header_text) { s.set_visible(false); }
        for (auto& s : combo_header_text) { s.set_visible(false); }
        for (auto& s : time_header_text) { s.set_visible(false); }
        for (auto& s : goal_or_limit_header_text) { s.set_visible(false); }
        for (auto& s : goal_or_limit_text) { s.set_visible(false); }
        for (auto& s : timer_sprites) { s.set_visible(false); }
        for (auto& ft : floating_texts) { ft.set_visible(false); }
        if (countdown_number_sprite.has_value()) { countdown_number_sprite.value().set_visible(false); }
        
        board_bg.set_visible(false);
        score_number_sprites.clear();
        combo_text_sprites.clear();
        spr_selector.set_visible(false);
        spr_selector_dirs.set_visible(false);
        bd.hide();
    }

    void show()
    {
        for (auto& s : score_header_text) { s.set_visible(true); s.set_palette(text_palette); }
        for (auto& s : combo_header_text) { s.set_visible(true); s.set_palette(text_palette); }
        for (auto& s : time_header_text) { s.set_visible(true); s.set_palette(text_palette); }
        for (auto& s : goal_or_limit_header_text) { s.set_visible(true); s.set_palette(text_palette); }
        for (auto& s : goal_or_limit_text) { s.set_visible(true); s.set_palette(text_palette); }
        for (auto& ft : floating_texts) { ft.set_visible(true); ft.set_palette(text_palette); }

        board_bg.set_visible(true);
        spr_selector.set_visible(true);
        spr_selector_dirs.set_visible(false);
        bd.show();
    }

    void reset()
    {
        // Reset game tracking state.
        state = game_state::countdown;
        score = 0;
        displayed_score = 0;
        combo = 1;
        board_animating = false;
        start_countdown_timer_frames = 60 * 3;
        timer_frames = 0;

        // Create new board.
        b.new_board();

        // Reset GUI.
        bd.reset();
        bd.animate_random_drop_all_in();
        floating_texts.clear();

        background.reset();
    }

    void newgame(game_mode _mode, int _level)
    {
        reset();
        this->mode = _mode;
        this->level = _level;
        this->score_goal = mode == game_mode::sprint ? levels::sprint[level] : 0;
        this->time_limit_frames = mode == game_mode::timeattack ? levels::timeattack[level] : 0;

        goal_or_limit_header_text.clear();
        text_generator.set_left_alignment();
        text_generator.generate(HEADER_X, +61, mode == game_mode::sprint ? "GOAL" : "LIMIT", goal_or_limit_header_text);
        text_generator.set_right_alignment();

        goal_or_limit_text.clear();
        auto goal_limit_value = mode == game_mode::sprint ? bn::to_string<5>(score_goal) : util::frames_to_time_string(time_limit_frames);
        text_generator.generate(VALUE_X, +70, goal_limit_value, goal_or_limit_text);
    }

    // Returns true if game is complete, based on different conditions depending on game mode.
    bool update()
    {
        if (state == game_state::gameover)
        {
            if (go_state == gameover_state::waiting_for_board_to_stop)
            {
                update_board();
                update_displayed_score();

                if (!board_animating)
                {
                    greyout_board();
                    go_state = gameover_state::waiting_for_board_to_greyout;
                }
            }
            else if (go_state == gameover_state::waiting_for_board_to_greyout)
            {
                // This step waits 1 second (60 frames) while we wait for the gems on the board to greyout 1 by 1.
                update_displayed_score();

                ++frames_waiting_for_board_to_greyout;

                // There are 5*6 gems (30) so we use the time waiting on this step halved to find the index of the next gem.
                auto number = frames_waiting_for_board_to_greyout / 2;

                // Search through the animation to find the next gem to grey out.
                int row = -1, col = -1;   // Set these to -1, so once they are set the loops break.
                for (int r = 0; r < board::rows && row == -1; ++r)
                {
                    for (int c = 0; c < board::cols && col == -1; ++c)
                    {
                        if (board_anim[r][c] == number)
                        {
                            // Setting these will break both loops.
                            row = r;
                            col = c;
                            
                            //goto found; // It's a game jam, why the hell not?!?! Exit both of the for loops

                            // This looked like it introduced some UNDEFINED BEHAVIOUR, causing matches to occur while the countdown
                            // was still happening, on gems that didn't even match...
                        }
                    }
                }

                //BN_ASSERT(false, "Index for board anim was not found, this should never get hit because of the goto above.");

                //found: // goto label
                bd.greyout(row, col);

                if (frames_waiting_for_board_to_greyout == 59)
                {
                    go_state = gameover_state::waiting_for_score_counter_to_stop;
                }
            }
            else if (go_state == gameover_state::waiting_for_score_counter_to_stop)
            {
                update_displayed_score();

                if (frames_waiting_for_score_to_finish_counting++ >= 60 && displayed_score == score)
                {
                    greyout_text();
                    go_state = gameover_state::waiting_for_menu_to_appear;
                }
            }
            else if (go_state == gameover_state::waiting_for_menu_to_appear)
            {
                if (frames_waiting_for_gameover_menu_to_appear++ >= 60)
                {
                    return true;
                }
            }
            else
            {
                BN_ASSERT(false, "Unknown go_state: ", (int)go_state);
            }

            animate_floating_texts();
            return false;
        }

        if (state == game_state::countdown)
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
                    music_util::maybe_play(bn::music_items::cirno);
                }

                --start_countdown_timer_frames;
            }
            else
            {
                countdown_number_sprite.reset();
                state = game_state::playing;
            }
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

        combo_text_sprites.clear();
        text_generator.generate(VALUE_X, -41, bn::format<4>("x{}", combo), combo_text_sprites);
        
        auto time_str = util::frames_to_time_string(timer_frames);
        timer_sprites.clear();
        text_generator.generate(VALUE_X, +49, time_str, timer_sprites);

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

