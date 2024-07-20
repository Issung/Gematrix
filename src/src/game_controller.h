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

#define HEADER_X +70    // x position for headers, with left-align generation.
#define VALUE_X +116    // x position for values, with right-align generation.

// Handles user input, passing it to `board`, and managing `board_drawer`'s animations, tracking score and combo.
class game_controller
{
private:
    board b;
    board_drawer bd = board_drawer(b);
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
    bn::sprite_palette_ptr active_palette = spr_selector.palette();
    bn::sprite_palette_ptr inactive_palette = create_palette(16, 16, 16);
    bn::optional<bn::sprite_ptr> countdown_number_sprite;
    bn::vector<floating_text, 64> floating_texts;
    int sel_row = 0;    // Current row of the selector.
    int sel_col = 0;    // Current column of the selector.
    int displayed_score = 0;
    bool displayed_score_bump = false;  // Bump the score display Y every frame it increments.
    int start_countdown_timer_frames = 0;    // How many frames remain until the game starts.
    bool animating = false;
    
    // Game mode variables.
    game_mode mode;
    ubyte level;
    int combo = 1;
    int score = 0;
    int timer_frames = 0;   // Amount of update frames since last reset.
    int score_goal = 0; // For sprint game mode, what is the player's goal score.
    int time_limit_frames = -1; // For time-attack game mode, what is the player's time limit (in frames).

    void animate_texts()
    {
        for(auto it = floating_texts.begin(), end = floating_texts.end(); it != end;)
        {
            floating_text& ft = *it;

            if(ft.update())
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

public: 
    game_controller()
    {
        text_generator.set_left_alignment();
        text_generator.generate(HEADER_X, -71, "SCORE", score_header_text);   // TODO: Fix Y position to align with gems border when added.
        text_generator.generate(HEADER_X, -50, "COMBO", combo_header_text);
        text_generator.generate(HEADER_X, +40, "TIME", time_header_text);
        // LIMIT/GOAL text is generated in newgame function.
        text_generator.set_right_alignment();

        text_generator_small.set_center_alignment();
    }

    game_mode get_mode() { return mode; }
    ubyte get_level() { return level; }

    // Get the metric relevant to the current gamemode (either score or frametime).
    int get_gamemode_metric()
    {
        if (mode == game_mode::sprint) { return timer_frames; }
        else if (mode == game_mode::timeattack) { return score; }
        else { BN_ASSERT(false, "Unknown game mode: ", (ubyte)mode); return 0; }
    }

    bn::string<8> get_gamemode_metric_display_string()
    {
        auto metric = get_gamemode_metric();
        if (mode == game_mode::sprint) { return util::frames_to_time_millis_string(metric); }
        else if (mode == game_mode::timeattack) { return bn::to_string<8>(metric); }
        else { BN_ASSERT(false, "Unknown game mode: ", (ubyte)mode); return bn::to_string<1>(0); }
    }

    void hide()
    {
        for (auto s : score_header_text) { s.set_visible(false); }
        for (auto s : combo_header_text) { s.set_visible(false); }
        for (auto s : time_header_text) { s.set_visible(false); }
        for (auto s : goal_or_limit_header_text) { s.set_visible(false); }
        for (auto s : goal_or_limit_text) { s.set_visible(false); }
        for (auto s : timer_sprites) { s.set_visible(false); }
        for (auto ft : floating_texts) { ft.set_visible(false); }
        if (countdown_number_sprite.has_value()) { countdown_number_sprite.value().set_visible(false); }
        
        score_number_sprites.clear();
        combo_text_sprites.clear();
        spr_selector.set_visible(false);
        bd.hide();
    }

    void show()
    {
        for (auto s : score_header_text) { s.set_visible(true); }
        for (auto s : combo_header_text) { s.set_visible(true); }
        for (auto s : time_header_text) { s.set_visible(true); }
        for (auto s : goal_or_limit_header_text) { s.set_visible(true); }
        for (auto s : goal_or_limit_text) { s.set_visible(true); }
        for (auto ft : floating_texts) { ft.set_visible(true); }

        spr_selector.set_visible(true);
        bd.show();
    }

    void reset()
    {
        // Reset game tracking state.
        score = 0;
        displayed_score = 0;
        combo = 1;
        animating = false;
        start_countdown_timer_frames = 60 * 3;
        timer_frames = 0;

        // Create new board.
        b.new_board();

        // Reset GUI.
        bd.reset();
        bd.animate_random_drop_all_in();
        floating_texts.clear();
    }

    void newgame(game_mode _mode, ubyte _level)
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
                bn::music_items::cirno.play(0.25);
            }

            --start_countdown_timer_frames;
        }
        else
        {
            countdown_number_sprite.reset();
        }

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
            if (!animating && (move_row != 0 || move_col != 0))
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

        auto selector_point = positions[sel_row][sel_col];
        spr_selector.set_position(selector_point);
        spr_selector.set_palette(animating ? inactive_palette : active_palette);

        auto board_animations_complete = bd.update();
        if (board_animations_complete)
        {
            // When the drawer says the animations are complete, the state variable is whatever it
            // was last animation, by knowing what it was was last animation we can tell what animations just completed.
            // Then we can decide what to do next.
            auto state = bd.state();

            // When a slide or gem drops complete, delete matches again.
            if (state == drawer_state::PlayingSlide || state == drawer_state::DroppingGems)
            {
                auto matches = b.delete_matches();

                for (auto m : matches)
                {
                    // TODO: Score adjustments:
                    // - Should each gem be worth worth its match size? E.g. A 4-of-a-kind each gem gives 4 points?
                    // - Should each match increase the combo rather than each play_matches?
                    auto points_per_gem = m.positions.size() * combo;
                    auto points_for_match = points_per_gem * m.positions.size();
                    score += points_for_match;
                    
                    bn::sound_items::match.play();

                    // TODO: Small innacuracy in displayed floating scores when 1 gem is used in 2 matches, the user
                    // only sees the top-most point for a single match.
                    for (auto p : m.positions)
                    {
                        auto palette = bd.colors[(uint8_t)m.type];
                        auto ft = floating_text(positions[p.row][p.col], palette, points_per_gem);
                        floating_texts.push_back(ft);
                    }
                }

                bd.play_matches(matches);
                combo += 1;
            }
            // After matches are destroyed, drop gems.
            else if (state == drawer_state::DestroyingMatches)
            {
                auto drops = b.drop_gems();
                bd.play_drops(drops);
            }
            else // State switches to `Waiting` wen play_matches() is called with an empty list.
            {
                animating = false;
                combo = 1;
            }
        }
        else
        {
            animating = true;
        }

        displayed_score_bump = false;
        if (displayed_score < score)
        {
            ++displayed_score;
        }
        else if (!displayed_score_bump) // Equal
        {
            displayed_score_bump = true;
        }

        // OPTIMISATION: Can optimise by only re-generating sprites if the displayed_score is different from last frame.
        // 6 characters can fit, before overflowing onto the game board.
        score_number_sprites.clear();
        text_generator.generate(VALUE_X, -64 + (displayed_score_bump ? 2 : 0), bn::to_string<32>(displayed_score), score_number_sprites);   // TODO: Fix Y position to align with gems border when added.

        combo_text_sprites.clear();
        text_generator.generate(VALUE_X, -41, bn::format<4>("x{}", combo), combo_text_sprites);
        
        auto time_str = util::frames_to_time_string(timer_frames);

        timer_sprites.clear();
        text_generator.generate(VALUE_X, +49, time_str, timer_sprites);

        if (start_countdown_timer_frames == 0)
        {
            ++timer_frames;
        }

        animate_texts();

        // Check for game-end conditions.
        if (
            (mode == game_mode::sprint && score >= score_goal)
            || (mode == game_mode::timeattack && timer_frames > time_limit_frames)
        )
        {
            return true;
        }

        return false;
    }
};

