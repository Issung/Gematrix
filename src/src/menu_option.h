#pragma once

#include "bn_string.h"
#include "menu_option_key.h"

class menu_option
{
public:
    // Must be long enough for records display (name + score).
    constexpr static int OPTION_TEXT_MAX_LENGTH = 12;

    bn::string<OPTION_TEXT_MAX_LENGTH> text;    // Text to display on screen for this menu item.
    menu_option_key key;   // Key to identify the option by so that when selected appropriate actions can be taken.
    
    menu_option(bn::string_view _text, menu_option_key _key) : text(_text), key(_key)
    {

    }
};