#ifndef BOARD_DRAWER
#define BOARD_DRAWER

#include "bn_core.h"
#include "bn_log.h"
#include "bn_sprite_items_gem.h"
#include "bn_sprite_palette_item.h"
#include "bn_sprite_palette_ptr.h"
#include "bn_sprite_ptr.h"
#include "bn_vector.h"
#include "bn_math.h"
#include "bn_sprite_actions.h"
#include "bn_list.h"
#include "board.h"

// TODO: Do we have to define this here? T-T It's being used in anim_slide.
static bn::fixed_point positions[board::rows][board::cols]; // The point of each drawn sprite, saved for use by the selector.

/*enum anim_type 
{
    slide
};

class anim
{
public:
    anim_type type;
};*/

enum drawer_state
{
    Waiting,    // Waiting for player input.
    PlayingSlide,  // Animating the player's input.
    DestroyingMatches, // Animating matches being destroyed.
    DroppingGems,   // Animating gems dropping.
};

// TODO: Change all of these to int8_t.
class anim_slide {
public:
    int from_row;
    int from_col;

    int to_row;
    int to_col;
    bn::sprite_move_to_action action;

    // Gem will be grabbed from `_to_row, _to_col`, color changed and moved to the `_from_row, _from_col` position, slid back 
    // to its `_to_row, _to_col` position.
    // Do everything so that when the animation ends, the sprite is where it needs to be with no further alterations.
    anim_slide(
        int _from_row, int _from_col,
        int _to_row, int _to_col,
        bn::sprite_ptr sprite,  // The sprite from _to_row, _to_col.
        bn::sprite_palette_ptr palette,  // The new palette for the gem at _to_row, _to_col.
        bool visible
    ) : from_row(_from_row), from_col(_from_col),
        to_row(_to_row), to_col(_to_col),
        action(
            determine_sprite(sprite, from_row, from_col),
            determine_duration(from_row, from_col, to_row, to_col),
            determine_to_position(to_row, to_col)
        )
    {
        sprite.set_visible(visible);
        sprite.set_palette(palette);

        auto debug_distance = determine_duration(from_row, from_col, to_row, to_col) / 10;
        //auto dbg_to = determine_to_position(to_row, to_col);
        BN_LOG("Created slide for ", from_row, ",", from_col, " to ", to_row, ",", to_col, ". Distance: ", debug_distance);
        //BN_LOG("Created slide for ", from.x(), ",", from.y(), " to ", dbg_to.x(), ",", dbg_to.y());
    }
    
private:
    // Horrible hack that allows us to set the sprite's position before we construct the tween action.
    static bn::sprite_ptr determine_sprite(bn::sprite_ptr sprite, int from_row, int from_col)
    {
        auto from = positions[bn::max(from_row, 0)][from_col];

        if (from_row < 0)
        {
            from = bn::fixed_point(from.x(), from.y() + (30 * from_row));
        }

        sprite.set_position(from);
        return sprite;
    }

    static int determine_duration(int from_row, int from_col, int to_row, int to_col)
    {
        constexpr int framesPerSquare = 10;
        auto distance = bn::abs((from_row - to_row) + (from_col - to_col));
        return bn::max(distance * framesPerSquare, 1);  // Setting a duration of 0 causes error, use min of 1.
    }

    static bn::fixed_point determine_to_position(int to_row, int to_col)
    {
        auto position = positions[to_row][to_col];
        return position;
    }
};

namespace
{
    const static bn::sprite_palette_ptr create_palette(int r, int g, int b)
    {
        auto color = bn::color(r, g, b);
        bn::color colors[16];
        for (int i = 0; i < 16; i++)
        {
            colors[i] = color;
        }
        auto span = bn::span<bn::color>(colors);
        auto palette = bn::sprite_palette_item(span, bn::bpp_mode::BPP_4);
        auto palette_ptr = palette.create_palette();
        return palette_ptr;
    }
}


// Class responsible for handling board drawing (including animations).
class board_drawer
{
private: 
    board& b;
    bn::vector<bn::sprite_ptr, board::total_gems> gem_sprites;  // Each gem sprite, can be accessed with `(row * board::cols) + col`.
    bn::list<anim_slide, board::total_gems> slides;
    // Move to ROM to avoid ram usage. https://gvaliente.github.io/butano/faq.html
    const bn::sprite_palette_ptr colors[board::max_colors] =
    {
        create_palette(31, 0, 0),   // Red
        create_palette(0, 31, 0),   // Green
        create_palette(0, 0, 31),   // Blue
        create_palette(31, 0, 31),  // Purple
        create_palette(31, 16, 0),  // Orange
        create_palette(31, 31, 31), // White (Wildcard)
        // No color for empty at the moment, just make the sprite invisible.
    };
    drawer_state current_state;
public:
    board_drawer(board& _b) : b(_b)
    {
        for (int r = 0; r < board::rows; r++)
        {
            for (int c = 0; c < board::cols; c++)
            {
                auto x = (30 * c) - 100;
                auto y = (30 * r) - 60;
                positions[r][c] = bn::fixed_point(x, y);
                auto gem_sprite = bn::sprite_items::gem.create_sprite(x, y);
                auto palette_index = (uint8_t)b.gems[r][c];
                auto palette = colors[palette_index];
                gem_sprite.set_palette(palette);

                gem_sprites.push_back(gem_sprite);
                BN_LOG("Made gem r: ", r, " c: ", c);
            }
        }

        BN_LOG("Finished constructing board_drawer.");
    }

    drawer_state state()
    {
        return current_state;
    }

    bool animating()
    {
        return slides.size() > 0;
    }

    // Slide gems between row/col A and row/col B.
    void slide(
        int row_a, int col_a,
        int row_b, int col_b
    )
    {
        current_state = drawer_state::PlayingSlide;

        auto sprite_a = gem_sprites[(row_a * board::cols) + col_a];
        auto type_a = b.gems[row_a][col_a];
        auto palette_a = colors[(int)type_a];
        slides.push_back(anim_slide(row_b, col_b, row_a, col_a, sprite_a, palette_a, type_a != gem_type::Empty));

        auto sprite_b = gem_sprites[(row_b * board::cols) + col_b];
        auto type_b = b.gems[row_b][col_b];
        auto palette_b = colors[(int)type_b];
        slides.push_back(anim_slide(row_a, col_a, row_b, col_b, sprite_b, palette_b, type_b != gem_type::Empty));
    }

    // Alters state, to either be waiting or destroying matches depending if there was any.
    void play_matches(match_collection& matches)
    {
        if (matches.size() == 0)
        {
            current_state = drawer_state::Waiting;
        }
        else
        {
            // TODO: Actually implement animations for destroying, probably just a shrink animation for now?
            current_state = drawer_state::DestroyingMatches;

            for (auto m : matches)
            {
                for (auto pos : m.positions)
                {
                    gem_sprites[(pos.row * board::cols) + pos.col].set_visible(false);
                }
            }

            matches.clear();
        }
    }

    void play_drops(gem_drops_collection& drops)
    {
        current_state = drawer_state::DroppingGems;

        // Create animation for each drop.
        for (auto drop : drops)
        {
            auto spr = gem_sprites[(drop.to_row * board::cols) + drop.col];
            auto palette = colors[(uint8_t)drop.type];
            slides.push_back(anim_slide(drop.from_row, drop.col, drop.to_row, drop.col, spr, palette, drop.type != gem_type::Empty));
        }
    }

    // Returns true if animations are complete.
    bool update()
    {
        auto it = slides.begin();
        auto end = slides.end();
        auto i = 0;
        bool didErase = false;
        while (it != end)
        {
            auto done = it->action.done();
            //BN_LOG("Processing tween index: ", i, ". Done: ", done);

            if (done == false)
            {
                it->action.update();
                ++it;
            }
            else
            {
                // Remove from list.
                it = slides.erase(it);
                didErase = true;
            }

            i++;
        }

        return slides.size() == 0;
    }

    void redraw_all_gems()
    {
        for (int r = 0; r < board::rows; ++r)
        {
            for (int c = 0; c < board::cols; ++c)
            {
                auto gem_sprite = gem_sprites[(r * board::cols) + c];
                auto gem_value = b.gems[r][c];

                if (gem_value == gem_type::Empty)
                {
                    gem_sprite.set_visible(false);
                }
                else
                {
                    auto palette = colors[(uint8_t)gem_value];
                    gem_sprite.set_palette(palette);
                    gem_sprite.set_visible(true);
                }
            }
        }
    }
};

#endif