#pragma once

enum class menu_option_key
{
    // Key can be used for options that don't do shit.
    noop,

    // Main Menu
    play,
    records,
    settings,

    // Play Menu
    play_sprint,         // Race to X amount of points.
    play_timeattack,    // How many points can be gotten in X amount of time.
    play_survival,       // How long can you stay alive against a decreasing lifespan by gaining more points.

    // Records Menu
    records_sprint,
    records_timeattack,

    // Settings Menu
    music_toggle,
    sfx_toggle,

    // Pause Menu
    resume,
    restart,
    quit,
};