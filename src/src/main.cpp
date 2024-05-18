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
#include "player.h"
#include "bn_sprite_items_gem.h"
#include "bn_sprite_items_selector.h"
#include "bn_fixed_point.h"
#include "bn_fixed.h"
#include "bn_sprite_palettes.h"
#include "bn_sprite_palette_ptr.h"
#include "bn_log.h"
#include "bn_random.h"

enum class gem_color {
    Red,
    Green,
    Blue,
    Purple,
    Orange
};

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

    // Restricted to 4bit color depth for now.
    // TODO: Figure out if there's better ways to do this.
    bn::sprite_palette_ptr create_palette(int r, int g, int b)
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
    const int total_gems = rows * cols;
    const int max_colors = 5;   // The amount of colors.
    auto rand = bn::random();
    int sel_row = 0;    // Current row of the selector.
    int sel_col = 0;    // Current column of the selector.

    bn::bg_palettes::set_transparent_color(bn::color(0, 0, 0));

    bn::sprite_text_generator text_generator(gj::fixed_32x64_sprite_font);

    text_generator.set_left_alignment();
    bn::vector<bn::sprite_ptr, 32> text_sprites;
    bn::vector<bn::sprite_ptr, total_gems> gem_sprites;

    //auto mi = bn::music_item(0);
    //mi.play(0.5);
    bn::music_items::cyberrid.play(0.05);

    int i = 0;

    auto spr_selector = bn::sprite_items::selector.create_sprite(0, 0);
    
    //auto red = bn::color(31, 0, 0);
    //bn::color reds[16];
    //for (int i = 0; i < 16; i++)
    //{
    //    reds[i] = red;
    //}
    //auto span = bn::span<bn::color>(reds);
    //auto red_palette = bn::sprite_palette_item(span, bn::bpp_mode::BPP_4);
    bn::sprite_palette_ptr colors[max_colors] = 
    {
        create_palette(31, 0, 0),   // Reg
        create_palette(0, 31, 0),   // Green
        create_palette(0, 0, 31),   // Blue
        create_palette(31, 0, 31),  // Purple
        create_palette(31, 16, 0),  // Orange
    };
    //auto red_palette_ptr = create_palette(31, 0, 0);

    bn::fixed_point points[rows][cols]; // The point of each drawn sprite, saved for use by the selector.
    gem_color gems[rows][cols];

    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            auto val = rand.get_int(max_colors);
            gems[r][c] = (gem_color)val;
        }
    }

    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            auto x = (30 * c) - 100;
            auto y = (30 * r) - 60;
            points[r][c] = bn::fixed_point(x, y);
            auto gem_sprite = bn::sprite_items::gem.create_sprite(x, y);
            auto palette_index = (int)gems[r][c];
            auto palette = colors[palette_index];
            gem_sprite.set_palette(palette);

            gem_sprites.push_back(gem_sprite);
            BN_LOG("Made gem c: ", r, " r: ", c);
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
            int move_row = 0;
            int move_col = 0;

            // TODO: Validation with error sfx and animation.
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

            auto current_gem = gems[sel_row][sel_col];
            auto target_gem = gems[sel_row + move_row][sel_col + move_col];
            gems[sel_row][sel_col] = target_gem;
            gems[sel_row + move_row][sel_col + move_col] = current_gem;
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

        for (int r = 0; r < rows; r++)
        {
            for (int c = 0; c < cols; c++)
            {
                auto gem_sprite = gem_sprites[(r * cols) + c];
                auto gem_value = gems[r][c];
                auto palette = colors[(int)gem_value];
                gem_sprite.set_palette(palette);
            }
        }

        //i = i + 1;
        //auto istr = bn::to_string<16>(i);
        //auto text = "Frame: " + istr;
        text_sprites.clear();
        text_generator.generate(+70, -73, combination, text_sprites);
        //auto draw_player = player.update(); // sprite only gets drawn if there's still a reference held to it? sprite_ptr is actually a smart pointer
        bn::core::update();
    }
}