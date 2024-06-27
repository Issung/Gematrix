#pragma once

#include "bn_core.h"
#include "bn_sram.h"
#include "bn_string.h"
#include "bn_bg_palettes.h"
#include "bn_sprite_text_generator.h"
#include <bn_log.h>

//#define FORMAT_TAG "BABABOOEY"
//#define FORMAT_TAG_LENGTH 9

// A single sprint score.
struct sprint_score
{
    bn::array<char, 3> name;
    int time_in_frames; // TODO: Store in a better way.
};

// A single score record for the timeattack mode.
struct timeattack_score
{
    bn::array<char, 3> name;
    int score;
};

struct sram_data
{
    bool enable_music = true;
    bool enable_sfx = true;
    
    // A sequence of characters that indicates the sram
    // has been formatted for the layout we desire. If this does not match
    // the expected string, then we know we need to format the sram.
    // Keep as the last field so that when we change the format it requires formatting again.
    bn::array<char, 10> format_tag;
};

class memory
{
private:
    inline static sram_data save_data;
public:
    static void init()
    {
        bn::sram::read(save_data);

        bn::array<char, 10> expected_format_tag;
        bn::istring_base expected_format_tag_istring(expected_format_tag._data);
        bn::ostringstream expected_format_tag_stream(expected_format_tag_istring);
        expected_format_tag_stream.append("BABABOOEY");

        if (save_data.format_tag == expected_format_tag)
        {
            BN_LOG("Initialised save_data, was already formatted.");
        }
        else
        {
            BN_LOG("Initialised save_data, was not formatted.");

            bn::sram::clear(bn::sram::size());
            save_data = sram_data();
            save_data.format_tag = expected_format_tag;
            bn::sram::write(save_data);
            
            BN_LOG("SRAM Cleared.");
        }
    }

    static bool music_enabled()
    {
        return save_data.enable_music;
    }

    static void music_enabled_set(bool enabled)
    {
        save_data.enable_music = enabled;
    }

    static bool sfx_enabled()
    {
        return save_data.enable_sfx;
    }

    static void sfx_enabled_set(bool enabled)
    {
        save_data.enable_sfx = enabled;
    }

    static void save()
    {
        bn::sram::write(save_data);
    }
};