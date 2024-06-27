#pragma once

enum class menu_option_key
{
    // Main Menu
    play,
    ranks,
    settings,

    // Play Menu
    sprint,         // Race to X amount of points.
    time_attack,    // How many points can be gotten in X amount of time.
    survival,       // How long can you stay alive against a decreasing lifespan by gaining more points.

    // Settings Menu
    music_toggle,
    sfx_toggle,

    // Pause Menu
    resume,
    restart,
    quit,
};