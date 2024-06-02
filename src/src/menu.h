#pragma once

#include "menu_option.h"
#include "bn_vector.h"

constexpr static int LONGEST_OPTION_TEXT = 11;
constexpr static int MAX_OPTIONS_PER_SCREEN = 3;

template <class T>
class menu
{
public:
    bn::string_view title;  // TODO: Display the title on-screen.
    menu<T>* previous_menu = nullptr;
    bn::vector<menu_option<T>, MAX_OPTIONS_PER_SCREEN> options;

    menu(
        bn::string_view _title,
        menu<T>* _previous_menu = nullptr
    ) : title(_title),
        previous_menu(_previous_menu)
    {

    }
};