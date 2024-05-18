#include "bn_bg_palettes.h"
#include "bn_core.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_text_generator.h"
#include "bn_vector.h"
#include "bn_keypad.h"

#include "gj_big_sprite_font.h"
#include "bn_sprite_palette_item.h"
#include "bn_sstream.h"
#include "bn_string.h"
#include "bn_format.h"
#include "bn_music.h"
#include "bn_music_item.h"
#include "bn_music_items.h"
#include "bn_sound_items.h"
//#include "bn_sprite_items_caveman.h"
#include "player.h"
#include "bn_sprite_items_gem.h"
#include "bn_sprite_items_selector.h"
#include "bn_fixed_point.h"
#include "bn_fixed.h"
#include "bn_sprite_palettes.h"

namespace 
{
    void tet()
    {

    }

    bn::sprite_ptr draw_caveman()
    {
        bn::sprite_ptr caveman = bn::sprite_items::caveman.create_sprite(-50, 0);
        caveman.set_bg_priority(2);
        caveman.set_z_order(1);
        return caveman;
    }

    //int lerp(int a, int b, int frames, int frames_max) {
    //    auto f = bn::fixed::from_data(frames);
    //    //auto fm = bn::fixed::from_data(frames_max);
    //    auto progress = f.division(frames_max);
    //    float progress = frames / frames_max;
    //    return a + (b - a) * progress;
    //}
}

int main()
{
    bn::core::init();

    const int cols = 6;   // The amount of columns to draw.
    const int rows = 5;   // The amount of rows to draw.
    int sel_row = 0;    // Current row of the selector.
    int sel_col = 0;    // Current column of the selector.

    bn::bg_palettes::set_transparent_color(bn::color(0, 0, 0));

    bn::sprite_text_generator text_generator(gj::fixed_32x64_sprite_font);

    text_generator.set_left_alignment();
    bn::vector<bn::sprite_ptr, 32> text_sprites;
    bn::vector<bn::sprite_ptr, rows*cols> gem_sprites;

    //auto mi = bn::music_item(0);
    //mi.play(0.5);
    bn::music_items::cyberrid.play(0.05);

    int i = 0;

    Player player;

    auto spr_selector = bn::sprite_items::selector.create_sprite(0, 0);
    
    auto red = bn::color(31, 0, 0);
    auto span = bn::span<const bn::color>(&red, 16);
    auto red_palette = bn::sprite_palette_item(span, bn::bpp_mode::BPP_4);

    bn::fixed_point points[rows][cols]; // The point of each drawn sprite, saved for use by the selector.

    for (int c = 0; c < cols; c++)
    {
        for (int r = 0; r < rows; r++)
        {
            auto x = (30 * c) - 100;
            auto y = (30 * r) - 60;
            points[r][c] = bn::fixed_point(x, y);
            auto gem_sprite = bn::sprite_items::gem.create_sprite(x, y);
            gem_sprite.set_palette(red_palette);

            gem_sprites.push_back(gem_sprite);
        }
    }




    while (true)
    {
        //if (bn::keypad::a_pressed() || bn::keypad::b_pressed() || bn::keypad::l_pressed() || bn::keypad::r_pressed())
        //{
        //    bn::sound_items::up.play(0.6);
        //}
        //if (bn::keypad::a_released() || bn::keypad::b_released() || bn::keypad::l_released() || bn::keypad::r_released())
        //{
        //    bn::sound_items::down.play(1);
        //}

        if (bn::keypad::a_held())
        {
            int move_row;
            int move_col;

            // TODO: Validation with error sfx and animation?
            if (bn::keypad::left_pressed() && sel_col > 0)
            {
                move_col = -1;
            }
            else if (bn::keypad::right_pressed() && sel_col < cols - 1)
            {
                move_col = +1;
            }

            if (bn::keypad::up_pressed() && sel_row > 0)
            {
                move_row = -1;
            }
            else if (bn::keypad::down_pressed() && sel_row < rows - 1)
            {
                move_row = +1;
            }
        }
        else
        {
            if (bn::keypad::left_pressed() && sel_col > 0)
            {
                sel_col -= 1;
            }
            else if (bn::keypad::right_pressed() && sel_col < cols - 1)
            {
                sel_col += 1;
            }

            if (bn::keypad::up_pressed() && sel_row > 0)
            {
                sel_row -= 1;
            }
            else if (bn::keypad::down_pressed() && sel_row < rows - 1)
            {
                sel_row += 1;
            }
        }

        //auto dc1 = draw_caveman();

        auto a = bn::keypad::a_held() ? "A" : "a";
        auto b = bn::keypad::b_held() ? "B" : "b";
        auto l = bn::keypad::l_held() ? "L" : "l";
        auto r = bn::keypad::r_held() ? "R" : "r";
        //auto combination = bn::format<7>("{} {} {} {}", a, b, l, r);
        auto combination = "SCORE";

        auto selector_point = points[sel_row][sel_col];
        spr_selector.set_position(selector_point);

        //i = i + 1;
        //auto istr = bn::to_string<16>(i);
        //auto text = "Frame: " + istr;
        text_sprites.clear();
        text_generator.generate(+70, -73, combination, text_sprites);
        auto draw_player = player.update(); // sprite only gets drawn if there's still a reference held to it? sprite_ptr is actually a smart pointer
        bn::core::update();
    }
}