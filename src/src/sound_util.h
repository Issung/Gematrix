#pragma once

#include "bn_sound.h"

// Even when SFX are turned off in game settings they are all still played, the m
// master volume is just set to one. Call this function whenever changing the setting.
class sound_util
{
public:
    static void set_sound_enabled(bool enable_sound)
    {
        bn::sound::set_master_volume(enable_sound ? 1 : 0);
    }
};