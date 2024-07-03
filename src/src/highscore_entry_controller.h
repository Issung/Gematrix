#pragma once

#include "memory.h"
#include "bn_array.h"
#include "bn_keypad.h"
#include "bn_vector.h"
#include "gj_big_sprite_font.h"

#define NAME_ALLOWED_CHARS_COUNT 27
static constexpr char name_allowed_chars[NAME_ALLOWED_CHARS_COUNT] = {
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
    bn::vector<bn::sprite_ptr, RECORD_NAME_LENGTH> menu_title_sprites;
    int position = 0;   // Current cursor position in the name (0 to 2).
    int chars[RECORD_NAME_LENGTH] = { 0, 0, 0 };

public:
    // Build name array from the user's input, use when `update()` returns `true`.
    bn::array<char, RECORD_NAME_LENGTH> build_name_array()
    {
        bn::array<char, RECORD_NAME_LENGTH> name = {name_allowed_chars[chars[0]], name_allowed_chars[chars[1]], name_allowed_chars[chars[2]]};
        BN_LOG("NAME ARRAY RESULT: ", name[0], name[1], name[2]);
        return name;
    }

    bn::string_view build_name_string()
    {
        auto array = build_name_array();
        auto str = bn::string<4>(array.data(), 3);
        BN_LOG("NAME STRING RESULT INNER: ", str);
        return str;
    }

    // Returns `false` if the user is still editing, returns `true` when the user confirms.
    bool update()
    {                  
        menu_title_sprites.clear();

        auto name_string = build_name_string();
        BN_LOG("NAME STRING RESULT OUTER:", name_string);

        text_generator.generate(0, 60, name_string, menu_title_sprites);

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
            chars[position] = (chars[position] == NAME_ALLOWED_CHARS_COUNT - 1) ? 0 : (chars[position] + 1);
        }
        else if (bn::keypad::down_pressed())
        {
            chars[position] = (chars[position] == 0) ? (NAME_ALLOWED_CHARS_COUNT - 1) : (chars[position] - 1);
        }
        else if (bn::keypad::a_pressed())
        {
            return true;
        }

        return false;
    }
};