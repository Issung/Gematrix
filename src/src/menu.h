#pragma once

#include "menu_option.h"
#include "bn_vector.h"
#include "menu_option_key.h"
#include "bn_optional.h"
#include "util.h"

constexpr static int LONGEST_TITLE_TEXT = 20;   // RECORDS (TIMEATTACK)
constexpr static int LONGEST_OPTION_TEXT = 11;
constexpr static int MAX_OPTIONS_PER_SCREEN = 5;
constexpr static int OPTIONS_Y_GAP = 20;    // Y position gap between menu options

// A menu contains a title, options and optionally a reference to a previous menu.
class menu
{
public:
    // We use string_view here because
    bn::string_view title;
    menu* previous_menu = nullptr;
    bn::vector<menu_option, MAX_OPTIONS_PER_SCREEN> options;

    // Is this menu interactable or is it just meant to display data.
    // E.g. Records or credits.
    bool interactable = true;

    // The below optionals are here for the hiscore screen, it needs to display more things than just menu options, that
    // aren't selectable, so they will use the options sprites beyond `options_count` and set a custom y position.

    // Optionally set the amount of options in `options` vector, rather than using the vector size.
    // So that other options sprites (beyond the count) can be used for other things (like score display).
    bn::optional<int> options_count;

    // Where to position menu options, position them in the middle if unset.
    // Used so other things can be displayed in other positions (like scores).
    bn::optional<int> options_y_position;

    menu(
        bn::string_view _title,
        menu* _previous_menu = nullptr
    ) : title(_title),
        previous_menu(_previous_menu)
    {

    }

    // Get the amount of options to draw.
    int get_options_count()
    {
        auto count = options_count.has_value() ? options_count.value() : options.size();
        return count;
    }

    // Get the y position the first menu option should be drawn at (going downwards from there).
    int get_y_position()
    {
        if (options_y_position.has_value())
        {
            return options_y_position.value();
        }

        // Desired y start positions (increments of half of OPTIONS_Y_GAP):
        //  1 item: 0
        //  2 items: -10
        //  3 items: -20
        //  4 items: -30
        //  5 items: -25 (break the formula because it gets too close to the title).
        auto count = get_options_count();
        return count == 5 ? -25 : (-((count - 1) * (OPTIONS_Y_GAP / 2))) + 5;
    }
};