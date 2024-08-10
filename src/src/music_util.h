#pragma once

#include "bn_random.h"
#include "bn_music.h"
#include "bn_music_item.h"
#include "memory.h"

// Unlike SFX we cannot just set an overall master volume for music, so we have to
// decide to either play or not play music tracks.
class music_util
{
private:
    constexpr static bn::fixed music_volume = 0.25;
    static bn::random rand;
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
    // Play random game music if music is enabled.
    // Use this function instead of playing music directly.
    static void play_random()
    {
        if (memory::save_data.enable_music)
        {
            auto i = rand.get_int(5);

            // Alphabetically sort by music name.
            if (i == 0) bn::music_items::mu_4_rndd.play(music_volume);
            else if (i == 1) bn::music_items::mu_biotech.play(music_volume);
            else if (i == 2) bn::music_items::mu_cirno.play(music_volume);
            else if (i == 3) bn::music_items::mu_fckdarules.play(music_volume);
            else if (i == 4) bn::music_items::mu_l3v3l_33.play(music_volume);
            else BN_ASSERT(false, "Invalid music index ", i);
        }
    }
};

bn::random music_util::rand = bn::random();