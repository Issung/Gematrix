#pragma once

#include "memory.h"
#include "bn_log.h"
#include "bn_array.h"
#include "bn_keypad.h"
#include "bn_vector.h"
#include "bn_optional.h"
#include "bn_sprite_ptr.h"
#include "gj_big_sprite_font.h"

#define NAME_ALLOWED_CHARS_COUNT 27
constexpr char name_allowed_chars[NAME_ALLOWED_CHARS_COUNT] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G',
    'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U',
    'V', 'W', 'X', 'Y', 'Z', '_'
};

// This class handles input & display for the highscore name entry GUI.
class highscore_entry_controller
{
private:
    bn::sprite_text_generator text_generator = bn::sprite_text_generator(gj::fixed_32x64_sprite_font);
    bn::vector<bn::sprite_ptr, RECORD_NAME_LENGTH> name_sprites;
    bn::optional<bn::sprite_ptr> up_arrow;
    bn::optional<bn::sprite_ptr> down_arrow;
    int position = 0;   // Current cursor position in the name (0 to 2).
    int chars[RECORD_NAME_LENGTH] = { 0, 0, 0 };

    // Convert an actual character, to an allowed character index for the array above.
    // Range 0 - 25 = A - Z, and 26 = _.
    int char_to_allowed_char_index(char c)
    {
        int ret = 
            (c == '_') ? NAME_ALLOWED_CHARS_COUNT - 1 :
            (c >= 'A' && c <= 'Z') ? c - 'A' :
            256;
        
        BN_ASSERT(ret != 256, "Character is not allowed: ", c);

        return ret;
    }

public:
    highscore_entry_controller()
    {
        BN_LOG("highscore_entry_controller");

        text_generator.set_center_alignment();
        text_generator.set_one_sprite_per_character(true);

        // Use the hat character as up/down arrows lol
        up_arrow = text_generator.generate<1>(0, 55, "^")[0];
        down_arrow = text_generator.generate<1>(0, 65, "^")[0];
        down_arrow->set_rotation_angle(180);

        auto last_name = memory::get_last_name();
        chars[0] = char_to_allowed_char_index(last_name[0]);
        chars[1] = char_to_allowed_char_index(last_name[1]);
        chars[2] = char_to_allowed_char_index(last_name[2]);
    }

    // Build name array from the user's input, use when `update()` returns `true`.
    bn::array<char, RECORD_NAME_LENGTH> build_name_array()
    {
        return {name_allowed_chars[chars[0]], name_allowed_chars[chars[1]], name_allowed_chars[chars[2]]};
    }

    // Hide the highscore entry, also setup state so next display is in the state the user expects.
    void hide()
    {
        position = 0;
        name_sprites.clear();
        up_arrow->set_visible(false);
        down_arrow->set_visible(false);
    }

    // Returns `false` if the user is still editing, returns `true` when the user confirms.
    bool update()
    {
        name_sprites.clear();

        auto name_array = build_name_array();
        auto name_string = bn::string<RECORD_NAME_LENGTH>(name_array.data(), RECORD_NAME_LENGTH);
        text_generator.generate(0, 60, name_string, name_sprites);

        up_arrow->set_visible(true);
        down_arrow->set_visible(true);

        if (bn::keypad::left_pressed() && position > 0)
        {
            position -= 1;
        }
        else if (bn::keypad::right_pressed() && position < RECORD_NAME_LENGTH - 1)
        {
            position += 1;
        }
        else if (bn::keypad::up_pressed())
        {
            chars[position] = (chars[position] == 0) ? (NAME_ALLOWED_CHARS_COUNT - 1) : (chars[position] - 1);
            up_arrow->set_visible(false);
        }
        else if (bn::keypad::down_pressed())
        {
            chars[position] = (chars[position] == NAME_ALLOWED_CHARS_COUNT - 1) ? 0 : (chars[position] + 1);
            down_arrow->set_visible(false);
        }
        else if (bn::keypad::a_pressed())
        {
            return true;
        }

        up_arrow->set_x(name_sprites[position].x() + 1);
        down_arrow->set_x(name_sprites[position].x() - 4);

        return false;
    }
};