#pragma once

#include "bn_vector.h"
#include "bn_sprite_actions.h"
#include "bn_sprite_text_generator.h"
#include "jg_variable_8x8_sprite_font.h"
#include "bn_string.h"
#include "bn_format.h"

static bn::sprite_text_generator text_generator_small = bn::sprite_text_generator(jg::variable_8x8_sprite_font);

class floating_text {
private:
    bn::vector<bn::sprite_ptr, 4> text_sprites;
    bn::sprite_move_to_action action;   // This action will be used for the first sprite, the others will be manually updated.
public:
    floating_text(
        bn::fixed_point _pos,
        bn::sprite_palette_ptr _palette,
        int _points
    ) : action(
        determine_sprite(_pos, _palette, _points),
        30, // Duration in frames
        determine_to_position(_pos)
    )
    {
    }
    
    // Returns true if done.
    bool update()
    {
        auto done = action.done();

        if (done == false)
        {
            action.update();

            for (int i = 1; i < text_sprites.size(); i++)
            {
                text_sprites[i].set_y(text_sprites[0].y());
            }

            return false;
        }

        return true;
    }

    void set_visible(bool visible)
    {
        for (auto s : text_sprites)
        {
            s.set_visible(visible);
        }
    }
private:
    // Horrible hack that allows us to set the sprite's position before we construct the tween action.
    bn::sprite_ptr determine_sprite(bn::fixed_point pos, bn::sprite_palette_ptr palette, int points)
    {
        auto str = bn::format<4>("+{}", points);
        text_generator_small.generate(pos, str, text_sprites);

        // TODO: Use nice palette that preserves black backdrop behind sprites otherwise readability becomes shitty.
        // Probably need to just take the gem_type in the constructor and use our own palettes in here.
        //for (auto s : text_sprites)
        //{
        //    s.set_palette(palette);
        //}

        return text_sprites[0];
    }

    static bn::fixed_point determine_to_position(bn::fixed_point pos)
    {
        // TODO: I have no idea why i have to add 10 to the x here, otherwise it floats diagonally towards upper-left.
        return bn::fixed_point(pos.x() + 10, pos.y() - 10);
    }
};