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

int main()
{
    bn::core::init();
    BN_LOG("INIT!");

    board b = board();
    board_drawer bd = board_drawer(b);

    int sel_row = 0;    // Current row of the selector.
    int sel_col = 0;    // Current column of the selector.

    bn::bg_palettes::set_transparent_color(bn::color(0, 0, 0));

    bn::vector<bn::sprite_ptr, 32> text_sprites;
    bn::sprite_text_generator text_generator(gj::fixed_32x64_sprite_font);
    text_generator.set_left_alignment();
    text_generator.generate(+70, -70, "SCORE", text_sprites);   // TODO: Fix Y position to align with gems border when added.

    //auto mi = bn::music_item(0);
    //mi.play(0.5);
    //bn::music_items::cyberrid.play(0.05);

    match_collection last_move_matches;

    auto spr_selector = bn::sprite_items::selector.create_sprite(0, 0);

    while (true)
    {
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

            if (move_row != 0 || move_col != 0)
            {
                // TODO: Implement a swap method on board and swap anim on board_drawer.
                auto current_gem = b.gems[sel_row][sel_col];
                auto target_gem = b.gems[sel_row + move_row][sel_col + move_col];
                b.gems[sel_row][sel_col] = target_gem;
                b.gems[sel_row + move_row][sel_col + move_col] = current_gem;

                bd.slide(sel_row, sel_col, sel_row, sel_col, sel_row + move_row, sel_col + move_col);
                bd.slide(sel_row + move_row, sel_col + move_col, sel_row + move_row, sel_col + move_col, sel_row, sel_col);

                // TODO: Optimise to only check for matches in altered rows/cols.
                last_move_matches = b.delete_matches();
                BN_LOG("Matches found: ", last_move_matches.size());
                for (int match_index = 0; match_index < last_move_matches.size(); ++match_index)
                {
                    bn::string<64> string;
                    bn::ostringstream string_stream(string);
                    string_stream.append("\tMatch: ");
                    string_stream.append(match_index);
                    string_stream.append(" indices: ");

                    auto match = last_move_matches[match_index];
                    for (int i = 0; i < match.positions.size(); ++i)
                    {
                        auto position = match.positions[i];

                        string_stream.append(position.row);
                        string_stream.append(",");
                        string_stream.append(position.col);
                        
                        if (i == match.positions.size() - 1)
                        {
                            string_stream.append(".");
                        }
                        else
                        {
                            string_stream.append(", ");
                        }
                    }
                    
                    BN_LOG(string_stream.view());
                }
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
        }

        // TODO: New board generation + drop in animation.
        /*if (bn::keypad::start_pressed())
        {
            for (int r = 0; r < board::rows; r++)
            {
                for (int c = 0; c < board::cols; c++)
                {
                    auto gem_sprite = gem_sprites[(r * board::cols) + c];
                    slides.push_back(anim_slide(r, c, r - board::rows, c, r, c, gem_sprite));
                }
            }
        }*/

        auto selector_point = positions[sel_row][sel_col];
        spr_selector.set_position(selector_point);

        if (bd.update())
        {
            if (last_move_matches.size() > 0)
            {
                bd.play_matches(last_move_matches);
            }
            else
            {
                bd.redraw_all_gems();
            }
        }
        bn::core::update();
    }
}