#pragma once

#include "util.h"

enum class menu_option_key : int
{
    // Key can be used for options that don't do shit.
    noop,

    // Main Menu
    play,
    records,
    settings,
    credits,

    // Play Menu
    play_sprint,         // Race to X amount of points.
    play_timeattack,    // How many points can be gotten in X amount of time.
    //play_survival,       // How long can you stay alive against a decreasing lifespan by gaining more points.

    // Play Menu (Level Select)
    play_level0,
    play_level1,
    play_level2,

    // Records Menu
    records_sprint,
    records_timeattack,

    // Records Menu (Level Select)
    records_level0,
    records_level1,
    records_level2,

    // Settings Menu
    toggle_music,
    toggle_sfx,

    // Pause Menu
    resume,
    restart,
    quit,
};