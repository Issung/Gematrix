#pragma once

// The state of the overall "game" (not the state of an actual game).
// Is the user currently on menus, ingame, paused, etc.
enum class overall_state
{
    menus,
    ingame,
    paused,
    hiscore,   // State for hiscore display & name entry.
};