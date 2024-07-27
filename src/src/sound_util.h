#pragma once

#include "bn_sound.h"

// Even when SFX are turned off in game settings they are all still played, the m
// master volume is just set to one. Call this function whenever changing the setting.
class sound_util
{
public:
    static void set_sound_enabled(bool enable_sound)
    {
        bn::fixed volume = enable_sound == true ? 1 : 0;
        bn::sound::set_master_volume(volume);
    }
};