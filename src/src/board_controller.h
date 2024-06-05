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
#include "player.h"

// Handles user input, passing it to `board`, and managing `board_drawer`'s animations, tracking score and combo.
class board_controller
{
private:
    board b;
    board_drawer bd = board_drawer(b);
    bn::sprite_text_generator text_generator = bn::sprite_text_generator(gj::fixed_32x64_sprite_font);
    bn::vector<bn::sprite_ptr, 5> score_text_sprites;  // Text sprites that just say "SCORE".
    bn::vector<bn::sprite_ptr, 32> score_number_sprites;
    bn::vector<bn::sprite_ptr, 4> combo_text_sprites;
    bn::sprite_ptr spr_selector = bn::sprite_items::selector.create_sprite(0, 0);
    bn::sprite_palette_ptr active_palette = spr_selector.palette();
    bn::sprite_palette_ptr inactive_palette = create_palette(16, 16, 16);
    int sel_row = 0;    // Current row of the selector.
    int sel_col = 0;    // Current column of the selector.
    int combo = 1;
    int score = 0;
    int displayed_score = 0;
    bool displayed_score_bump = false;  // Bump the score display Y every frame it increments.
    bool animating = false;
public: 
    board_controller()
    {
        text_generator.set_left_alignment();
        text_generator.generate(+70, -70, "SCORE", score_text_sprites);   // TODO: Fix Y position to align with gems border when added.
        text_generator.set_right_alignment();
    }

    void hide()
    {
        for (auto s : score_text_sprites) { s.set_visible(false); }
        score_number_sprites.clear();
        combo_text_sprites.clear();

        spr_selector.set_visible(false);
        bd.hide();
    }

    void show()
    {
        for (auto s : score_text_sprites) { s.set_visible(true); }
        spr_selector.set_visible(true);
        bd.show();
    }

    void reset()
    {
        score = 0;
        displayed_score = 0;
        combo = 1;
        animating = false;
        b.new_board();
        bd.redraw_all_gems();   // TODO: Animate board dropping row-by-row.
    }

    void update()
    {
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
            }
        }
        else
        {
            if (bn::keypad::left_pressed() && sel_col > 0)
            {
                sel_col -= 1;
            }
            else if (bn::keypad::right_pressed() && sel_col < board::cols - 1)
            {
                sel_col += 1;
            }

            if (bn::keypad::up_pressed() && sel_row > 0)
            {
                sel_row -= 1;
            }
            else if (bn::keypad::down_pressed() && sel_row < board::rows - 1)
            {
                sel_row += 1;
            }
            else if (bn::keypad::select_pressed())
            {
                bd.animate_random_drop_all_in();
            }
        }

        auto selector_point = positions[sel_row][sel_col];
        spr_selector.set_position(selector_point);
        spr_selector.set_palette(animating ? inactive_palette : active_palette);

        auto animations_complete = bd.update();
        if (animations_complete)
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
                    score += m.positions.size() * combo;
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
        text_generator.generate(+116, -55 + (displayed_score_bump ? 2 : 0), bn::to_string<32>(displayed_score), score_number_sprites);   // TODO: Fix Y position to align with gems border when added.

        combo_text_sprites.clear();
        text_generator.generate(+116, +55, bn::format<4>("x{}", combo), combo_text_sprites);
    }
};

