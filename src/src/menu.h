#pragma once

#include "menu_option.h"
#include "bn_vector.h"
#include "menu_option_key.h"

constexpr static int LONGEST_OPTION_TEXT = 11;
constexpr static int MAX_OPTIONS_PER_SCREEN = 3;

class menu
{
public:
    bn::string_view title;  // TODO: Display the title on-screen.
    menu* previous_menu = nullptr;
    bn::vector<menu_option, MAX_OPTIONS_PER_SCREEN> options;

    menu(
        bn::string_view _title,
        menu* _previous_menu = nullptr
    ) : title(_title),
        previous_menu(_previous_menu)
    {

    }
};