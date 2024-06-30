#pragma once

#include "bn_core.h"
#include "bn_sram.h"
#include "bn_string.h"
#include "bn_bg_palettes.h"
#include "bn_sprite_text_generator.h"
#include "bn_log.h"
#include "bn_list.h"
#include "util.h"

#define FORMAT_TAG "BABABOOEY"  // Expected character sequence to determine if SRAM is formatted or not.
#define FORMAT_TAG_LENGTH 9 // Length of the above format tag.
#define MAX_RECORDS 5    // Maximum amount of records to store per game mode.
#define RECORD_NAME_LENGTH 3   // Length of a name in a record.
#define RECORD_NAME bn::array<char, RECORD_NAME_LENGTH>

// A single record for the sprint mode.
struct record_sprint
{
    RECORD_NAME name;
    int time_in_frames;

    record_sprint()
    {
    }

    record_sprint(RECORD_NAME _name, int _time_in_frames) : name(_name), time_in_frames(_time_in_frames)
    {
    }
};

// A single record for the timeattack mode.
struct record_timeattack
{
    RECORD_NAME name;
    int score;

    record_timeattack()
    {
    }

    record_timeattack(RECORD_NAME _name, int _score) : name(_name), score(_score)
    {
    }
};

// Struct, so we can load it all from SRAM.
struct sram_data
{
    bool enable_music = true;
    bool enable_sfx = true;
    
    // TODO: Sprint records.
    bn::array<record_timeattack, MAX_RECORDS> timeattack_records; // Scores for timeattack mode, [0] is the highest score.

    // A sequence of characters that indicates the sram
    // has been formatted for the layout we desire. If this does not match
    // the expected string, then we know we need to format the sram.
    // Keep as the last field so that when we change the format it requires formatting again.
    bn::array<char, 10> format_tag;
};

class memory
{
private:
public:
    inline static sram_data save_data;  // Bit nasty having this public but we trust ourself not to do anything dirty like modifying it.
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
            auto name = bn::array<char, 3>();
            name[0] = 'T';
            name[1] = 'E';
            name[2] = 'T';
            save_data.timeattack_records[0] = record_timeattack(name, 1000);
            save_data.timeattack_records[1] = record_timeattack(name, 800);
            save_data.timeattack_records[2] = record_timeattack(name, 500);
            save_data.timeattack_records[3] = record_timeattack(name, 400);
            save_data.timeattack_records[4] = record_timeattack(name, 150);
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

    static bool is_record_timeattack(int score)
    {
        for (auto record : save_data.timeattack_records)
        {
            if (score > record.score)
            {
                return true;
            }
        }

        return false;
    }

    static void save_record_timeattack(record_timeattack record)
    {
       for (ubyte i = 0; i < MAX_RECORDS; ++i)
       {
            if (record.score > save_data.timeattack_records[i].score)
            {
                // Read the scores array backwards from end to current position.
                // Shifting the scores towards the end, overwriting the final score.
                for (int j = MAX_RECORDS - 1; j > i; --j)
                {
                    save_data.timeattack_records[j] = save_data.timeattack_records[j - 1];
                }

                // Insert the new score
                save_data.timeattack_records[i] = record;
                save();
                return;
            }
        }

        BN_ASSERT(false, "Tried to save a timeattack record that was not better than any existing records");
    }
};