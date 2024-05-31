#pragma once

#include "bn_string.h"

template <class T>
class menu_option
{
public:
    bn::string_view text;    // Text to display on screen for this menu item.
    T key;   // Key to identify the option by so that when selected appropriate actions can be taken.
    
    menu_option(bn::string_view _text, T _key) : text(_text), key(_key)
    {

    }
};