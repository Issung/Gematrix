#ifndef BOARD_DRAWER
#define BOARD_DRAWER

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

// TODO: Change all of these to int8_t.
class anim_slide {
public:
    // These are needed for slide animations that come from offscreen so we can reset the sprite back to its original position.
    int spr_row;
    int spr_col;

    int from_row;
    int from_col;

    int to_row;
    int to_col;
    bn::sprite_move_to_action action;

    anim_slide(
        int _spr_row, int _spr_col,
        int _from_row, int _from_col,
        int _to_row, int _to_col,
        bn::sprite_ptr sprite
    ) : 
        spr_row(_spr_row), spr_col(_spr_col),
        from_row(_from_row), from_col(_from_col),
        to_row(_to_row), to_col(_to_col),
        action(
            determine_sprite(sprite, from_row, from_col),
            determine_distance(from_row, from_col, to_row, to_col),
            determine_to_position(to_row, to_col)
        )
    {
        auto from = positions[bn::max(from_row, 0)][from_col];

        if (from_row < 0)
        {
            from = bn::fixed_point(from.x(), from.y() + (30 * from_row));
        }

        sprite.set_position(from);

        //auto debug_distance = determine_distance(from_row, from_col, to_row, to_col);
        //auto dbg_to = determine_to_position(to_row, to_col);
        //BN_LOG("Created slide for ", from_row, ",", from_col, " to ", to_row, ",", to_col, ". Distance: ", debug_distance);
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

    static int determine_distance(int from_row, int from_col, int to_row, int to_col)
    {
        auto distance = bn::abs((from_row - to_row) + (from_col - to_col)) * 10;
        return distance;
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
    const bn::sprite_palette_ptr colors[board::max_colors] =
    {
        create_palette(31, 0, 0),   // Red
        create_palette(0, 31, 0),   // Green
        create_palette(0, 0, 31),   // Blue
        create_palette(31, 0, 31),  // Purple
        create_palette(31, 16, 0),  // Orange
        create_palette(31, 31, 31),  // White (Wildcard)
        // No color for empty at the moment, just make the sprite invisible.
    };
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

    void slide(
        int spr_row, int spr_col,
        int from_row, int from_col,
        int to_row, int to_col
    )
    {
        auto current_gem_sprite = gem_sprites[(spr_row * board::cols) + spr_col];
        slides.push_back(anim_slide(spr_row, spr_col, from_row, from_col, to_row, to_col, current_gem_sprite));
    }

    void play_matches(match_collection& matches)
    {
        for (auto m : matches)
        {
            for (auto pos : m.positions)
            {
                gem_sprites[(pos.row * board::cols) + pos.col].set_visible(false);
            }
        }

        matches.clear();
        auto drops = b.drop_gems();

        // Create animation for each drop.
        for (auto drop : drops)
        {
            auto spr = gem_sprites[(drop.row * board::cols) + drop.col];
            slides.push_back(anim_slide(drop.row, drop.col, drop.from_row, drop.col, drop.to_row, drop.col, spr));
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
                // Reset gem pos & palette.
                auto sprite = it->action.sprite();
                sprite.set_position(positions[it->spr_row][it->spr_col]);
                auto gem_type = b.gems[it->spr_row][it->spr_col];
                if (gem_type == gem_type::Empty)    // TODO: Stop duplicating this logic from redraw_all_gems.
                {
                    sprite.set_visible(false);
                }
                else
                {
                    auto palette = colors[(uint8_t)gem_type];
                    sprite.set_palette(palette);
                    sprite.set_visible(true);
                }

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