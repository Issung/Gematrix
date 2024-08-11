#pragma once

#include "bn_seed_random.h"
#include "bn_music.h"
#include "bn_music_item.h"
#include "memory.h"

// Unlike SFX we cannot just set an overall master volume for music, so we have to
// decide to either play or not play music tracks.
class music_util
{
private:
    constexpr static bn::fixed music_volume = 0.25;
    static bn::seed_random rand;    // Don't want the music rotation to be the same everytime the game is played.
    static bool seed_set;

    // How many calls required to totally slowdown/fadeout the music.
    constexpr static int slowdown_frames = 300; // 5 seconds.
    constexpr static bn::fixed min_pitch = 0.5; // Min pitch allowed by butano.
    constexpr static bn::fixed min_volume = 0.0;
    constexpr static bn::fixed pitch_inc = min_pitch / slowdown_frames;   // Decrement to lower pitch from 1 to 0.5 over slowdown_frames.
    constexpr static bn::fixed vol_inc = music_volume / slowdown_frames;    // Decrement to lower volume from 1 to 0 over slowdown_frames.
public:
    // Set the seed for random track selection.
    // Can be called multiple times without error but will only use the first.
    static void set_seed(unsigned int frames_since_boot)
    {
        if (!seed_set)
        {
            rand.set_seed(frames_since_boot);
            seed_set = true;
        }
    }

    // Call this repeatedly to lower music pitch to minimum and vol to 0.
    static void slowdown()
    {
        if (bn::music::playing())
        {
            auto new_pitch = bn::max(min_pitch, bn::music::pitch() - pitch_inc);
            auto new_vol = bn::max(min_volume, bn::music::volume() - vol_inc);

            bn::music::set_pitch(new_pitch);
            bn::music::set_volume(new_vol);
        }
    }

    // Stop music if there is some playing.
    // Call this when the user turns off music.
    static void stop()
    {
        if (bn::music::playing())
        {
            bn::music::stop();
        }
    }

    // Pause music if there is some playing.
    static void pause()
    {
        if (bn::music::playing())
        {
            bn::music::pause();
        }
    }

    // Resume music if there is some paused.
    static void resume()
    {
        if (bn::music::paused())
        {
            bn::music::resume();
        }
    }

    // Play the menu music, if music is enabled.
    static void play_menu()
    {
        if (memory::save_data.enable_music)
        {
            // Don't restart the menu tune if its already playing.
            auto playing_music = bn::music::playing_item();
            if (!playing_music.has_value() || playing_music.value() != bn::music_items::mu_pms_are1)
            {
                bn::music_items::mu_pms_are1.play(music_volume);
            }
        }
    }

    // Play random game music if music is enabled.
    // Use this function instead of playing music directly.
    static void play_random()
    {
        if (memory::save_data.enable_music)
        {
            auto i = rand.get_unbiased_int(5);

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

bool music_util::seed_set = false;
bn::seed_random music_util::rand = bn::seed_random();