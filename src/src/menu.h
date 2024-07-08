#pragma once

#include "menu_option.h"
#include "bn_vector.h"
#include "menu_option_key.h"
#include "bn_optional.h"
#include "util.h"

constexpr static int LONGEST_OPTION_TEXT = 11;
constexpr static int MAX_OPTIONS_PER_SCREEN = 5;

// A menu contains a title, options and optionally a reference to a previous menu.
class menu
{
public:
    // We use string_view here because 
    bn::string_view title;
    menu* previous_menu = nullptr;
    bn::vector<menu_option, MAX_OPTIONS_PER_SCREEN> options;

    // The below optionals are here for the hiscore screen, it needs to display more things than just menu options, that
    // aren't selectable, so they will use the options sprites beyond `options_count` and set a custom y position.

    // Optionally set the amount of options in `options` vector, rather than using the vector size.
    // So that other options sprites (beyond the count) can be used for other things (like score display).
    bn::optional<ubyte> options_count;

    // Where to position menu options, position them in the middle if unset.
    // Used so other things can be displayed in other positions (like scores).
    bn::optional<sbyte> options_y_position;

    menu(
        bn::string_view _title,
        menu* _previous_menu = nullptr
    ) : title(_title),
        previous_menu(_previous_menu)
    {

    }

    ubyte get_options_count()
    {
        auto count = options_count.has_value() ? options_count.value() : (ubyte)options.size();
        return count;
    }

    sbyte get_y_position()
    {
        auto position = options_y_position.has_value() ? options_y_position.value() : (sbyte)-20;   // The default here on the end.
        return position;
    }
};