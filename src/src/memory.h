#pragma once

#include "bn_core.h"
#include "bn_sram.h"
#include "game_mode.h"
#include "bn_string.h"
#include "bn_bg_palettes.h"
#include "bn_sprite_text_generator.h"
#include "bn_log.h"
#include "bn_list.h"
#include "util.h"
#include "bn_sound.h"
#include "bn_music.h"
#include "sound_util.h"

#define FORMAT_TAG "BABABOOEY"  // Expected character sequence to determine if SRAM is formatted or not.
#define FORMAT_TAG_LENGTH 9 // Length of the above format tag.
#define MAX_RECORDS 5    // Maximum amount of records to store per game mode.
#define RECORD_NAME_LENGTH 3   // Length of a name in a record.
#define RECORD_NAME bn::array<char, RECORD_NAME_LENGTH>
#define SPRINT_RECORD_LIST bn::array<record_sprint, MAX_RECORDS>
#define LEVELS 3
#define TIMEATTACK_RECORD_LIST bn::array<record_timeattack, MAX_RECORDS>

enum class palette_setting
{
    og, // Original design for AGS-101 and GBM or emulation.
    lcd,    // Lcd palette for GBA / SP.
};

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
    bool enable_sfx = true;
    bool enable_music = true;
    palette_setting palette = palette_setting::og;
    
    // Arrays of arrays, for level selection, then records.
    // Levels (0 - 2), 0 is the lowest/fastest level, going upwards from there.
    // Records (0 - 4), 0 is the best score and worsens from there.
    bn::array<SPRINT_RECORD_LIST, LEVELS> records_sprint;
    bn::array<TIMEATTACK_RECORD_LIST, LEVELS> records_timeattack; // Scores for timeattack mode, [0] is the highest score.

    RECORD_NAME last_name;  // The last name used for a record, used to populate the name entry again.

    // A sequence of characters that indicates the sram
    // has been formatted for the layout we desire. If this does not match
    // the expected string, then we know we need to format the sram.
    // Keep as the last field so that when we change the format it requires formatting again.
    bn::array<char, 10> format_tag;
};

class memory
{
private:
    // Returns empty optional or the new record position 0 to 4 (1st to 5th).
    static bn::optional<int> is_record_sprint(int level, int time_in_frames)
    {
        for (int i = 0; i < MAX_RECORDS; ++i)
        {
            if (time_in_frames < save_data.records_sprint[level][i].time_in_frames)
            {
                return i;
            }
        }

        return bn::optional<int>();
    }

    // Returns empty optional or the new record position 0 to 4 (1st to 5th).
    static bn::optional<int> is_record_timeattack(int level, int score)
    {
        for (int i = 0; i < MAX_RECORDS; ++i)
        {
            if (score > save_data.records_timeattack[level][i].score)
            {
                return i;
            }
        }

        return bn::optional<int>();
    }

    static void insert_record_sprint(int level, record_sprint record)
    {
       for (int i = 0; i < MAX_RECORDS; ++i)
       {
            if (record.time_in_frames < save_data.records_sprint[level][i].time_in_frames)
            {
                // Read the scores array backwards from end to current position.
                // Shifting the scores towards the end, overwriting the final score.
                for (int j = MAX_RECORDS - 1; j > i; --j)
                {
                    save_data.records_sprint[level][j] = save_data.records_sprint[level][j - 1];
                }

                // Insert the new score
                save_data.records_sprint[level][i] = record;
                return;
            }
        }

        BN_ASSERT(false, "Tried to save a timeattack record that was not better than any existing records");
    }

    static void insert_record_timeattack(int level, record_timeattack record)
    {
       for (int i = 0; i < MAX_RECORDS; ++i)
       {
            if (record.score > save_data.records_timeattack[level][i].score)
            {
                // Read the scores array backwards from end to current position.
                // Shifting the scores towards the end, overwriting the final score.
                for (int j = MAX_RECORDS - 1; j > i; --j)
                {
                    save_data.records_timeattack[level][j] = save_data.records_timeattack[level][j - 1];
                }

                // Insert the new score
                save_data.records_timeattack[level][i] = record;
                return;
            }
        }

        BN_ASSERT(false, "Tried to save a timeattack record that was not better than any existing records");
    }
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
            BN_LOG("Memory save_data not formatted, initialising now.");

            bn::sram::clear(bn::sram::size());
            save_data = sram_data();
            // Populate defaults.
            save_data.enable_sfx = true;
            save_data.enable_music = true;
            save_data.palette = palette_setting::og;

            // TODO: Make up new scores & names.

            // Sprints LOWER IS BETTER
            // level 0
            save_data.records_sprint[0][0] = record_sprint(util::to_record_name("ISN"), 2307);  // 00:38.45
            save_data.records_sprint[0][1] = record_sprint(util::to_record_name("SND"), 2532);
            save_data.records_sprint[0][2] = record_sprint(util::to_record_name("ZAP"), 2760);
            save_data.records_sprint[0][3] = record_sprint(util::to_record_name("BOO"), 2905);
            save_data.records_sprint[0][4] = record_sprint(util::to_record_name("LST"), 3100);
            // level 1
            save_data.records_sprint[1][0] = record_sprint(util::to_record_name("ISN"), 5640);   // 1:34.06
            save_data.records_sprint[1][1] = record_sprint(util::to_record_name("BOB"), 6123);
            save_data.records_sprint[1][2] = record_sprint(util::to_record_name("YUM"), 6499);
            save_data.records_sprint[1][3] = record_sprint(util::to_record_name("KTY"), 6765);
            save_data.records_sprint[1][4] = record_sprint(util::to_record_name("CAT"), 7000);
            // level 2
            save_data.records_sprint[2][0] = record_sprint(util::to_record_name("ISN"), 14325);  // 3:58.76
            save_data.records_sprint[2][1] = record_sprint(util::to_record_name("EZY"), 14423);
            save_data.records_sprint[2][2] = record_sprint(util::to_record_name("QQQ"), 14642);
            save_data.records_sprint[2][3] = record_sprint(util::to_record_name("BAD"), 14890);
            save_data.records_sprint[2][4] = record_sprint(util::to_record_name("WIN"), 15100);
            // Time Attack HIGHER IS BETTER
            // level 0
            save_data.records_timeattack[0][0] = record_timeattack(util::to_record_name("ISN"), 1631);
            save_data.records_timeattack[0][1] = record_timeattack(util::to_record_name("ACE"), 1611);
            save_data.records_timeattack[0][2] = record_timeattack(util::to_record_name("GGG"), 1542);
            save_data.records_timeattack[0][3] = record_timeattack(util::to_record_name("BOT"), 1497);
            save_data.records_timeattack[0][4] = record_timeattack(util::to_record_name("MEH"), 1454);
            // level 1
            save_data.records_timeattack[1][0] = record_timeattack(util::to_record_name("ISN"), 3386);
            save_data.records_timeattack[1][1] = record_timeattack(util::to_record_name("ELF"), 3267);
            save_data.records_timeattack[1][2] = record_timeattack(util::to_record_name("BRE"), 3211);
            save_data.records_timeattack[1][3] = record_timeattack(util::to_record_name("OOP"), 3180);
            save_data.records_timeattack[1][4] = record_timeattack(util::to_record_name("HAX"), 3095);
            // level 2
            save_data.records_timeattack[2][0] = record_timeattack(util::to_record_name("ISN"), 6680);
            save_data.records_timeattack[2][1] = record_timeattack(util::to_record_name("VIP"), 6579);
            save_data.records_timeattack[2][2] = record_timeattack(util::to_record_name("NAN"), 6453);
            save_data.records_timeattack[2][3] = record_timeattack(util::to_record_name("HAX"), 6311);
            save_data.records_timeattack[2][4] = record_timeattack(util::to_record_name("BUG"), 6282);
            save_data.last_name = util::to_record_name("AAA");
            save_data.format_tag = expected_format_tag;
            bn::sram::write(save_data);

            BN_LOG("Memory save_data initialised.");
        }

        sound_util::set_sound_enabled(save_data.enable_sfx);
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

    static palette_setting palette()
    {
        return save_data.palette;
    }

    static void palette_set(palette_setting new_palette)
    {
        save_data.palette = new_palette;
    }

    // Write `save_data` to SRAM.
    static void save()
    {
        bn::sram::write(save_data);
    }

    // Return record position (0 - 4  (1st to 5th)) for the relevant `game_mode` (sprint / timeattack).
    static bn::optional<int> is_record(game_mode mode, int level, int frametime_or_score)
    {
        if (mode == game_mode::sprint)
        {
            return is_record_sprint(level, frametime_or_score);
        }
        else if (mode == game_mode::timeattack)
        {
            return is_record_timeattack(level, frametime_or_score);
        }
        else
        {
            BN_ASSERT(false, "Unknown game mode: ", (int)mode);
            return bn::optional<int>();
        }
    }

    // Save sprint / timeattack record depending on `mode`.
    static void save_record(game_mode mode, int level, RECORD_NAME name, int frametime_or_score)
    {
        if (mode == game_mode::sprint)
        {
            insert_record_sprint(level, record_sprint(name, frametime_or_score));
        }
        else if (mode == game_mode::timeattack)
        {
            insert_record_timeattack(level, record_timeattack(name, frametime_or_score));
        }
        else
        {
            BN_ASSERT(false, "Unknown game mode: ", (int)mode);
        }

        save_data.last_name = name;
        save();
    }

    static RECORD_NAME get_last_name()
    {
        return save_data.last_name;
    }
};