#pragma once

#include "bn_music.h"
#include "bn_music_item.h"
#include "memory.h"

// Unlike SFX we cannot just set an overall master volume for music, so we have to
// decide to either play or not play music tracks.
class music_util
{
private:
    constexpr static bn::fixed music_volume = 0.25;
public:
    // Stop music if there is some playing.
    // Call this when the user turns off music.
    static void maybe_stop()
    {
        if (bn::music::playing())
        {
            bn::music::stop();
        }
    }

    // Pause music if there is some playing.
    static void maybe_pause()
    {
        if (bn::music::playing())
        {
            bn::music::pause();
        }
    }

    // Resume music if there is some paused.
    static void maybe_resume()
    {
        if (bn::music::paused())
        {
            bn::music::resume();
        }
    }

    // Play music if the setting is enabled.
    // Use this function instead of playing music directly.
    static void maybe_play(bn::music_item item)
    {
        if (memory::save_data.enable_music)
        {
            item.play(music_volume);
        }
    }
};