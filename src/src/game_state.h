#pragma once

enum class game_state
{
    menus,
    ingame,
    paused,
    gameover,   // State for game finish and score display, and name entry if user earned hiscore.
};