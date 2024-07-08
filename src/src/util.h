#pragma once

#include "bn_array.h"
#include "bn_string.h"

#define ubyte uint8_t   // Unsigned byte
#define sbyte int8_t    // Signed byte

class util
{
public:
    // Convert a frame count to a time string in the format "mm:ss" with leading zeroes.
    // Based on a 60fps assumption (60 frames = 1 second).
    static bn::string<5> frames_to_time_string(int frame_count)
    {
        int seconds = frame_count / 60;
        int minutes = seconds / 60;
        seconds = seconds - (minutes * 60);

        // Build time string in format "01:23".
        // TODO: Refactor string formatting to util class method.
        // TODO: Make time count downards if in time-attack mode.
        bn::string<5> timer_str;
        bn::ostringstream string_stream(timer_str);

        if (minutes < 10) string_stream.append("0");    // Append leading zero if not double digits.
        string_stream.append(minutes);
        string_stream.append(":");
        if (seconds < 10) string_stream.append("0");    // Append leading zero if not double digits.
        string_stream.append(seconds);
        
        return timer_str;
    }

    // Convert a frame count to a time string in the format "mm:ss.xx" with leading zeroes.
    // Based on a 60fps assumption (60 frames = 1 second).
    static bn::string<8> frames_to_time_millis_string(int frame_count)
    {
        int total_seconds = frame_count / 60;
        int minutes = total_seconds / 60;
        int seconds = total_seconds % 60;
        int milliseconds = (frame_count % 60) * 100 / 60;

        // Build time string in format "01:23.45".
        // TODO: Refactor string formatting to util class method.
        // TODO: Make time count downwards if in time-attack mode.
        bn::string<8> timer_str;
        bn::ostringstream string_stream(timer_str);

        if (minutes < 10) string_stream.append("0");    // Append leading zero if not double digits.
        string_stream.append(minutes);
        string_stream.append(":");
        if (seconds < 10) string_stream.append("0");    // Append leading zero if not double digits.
        string_stream.append(seconds);
        string_stream.append(".");
        if (milliseconds < 10) string_stream.append("0");    // Append leading zero if not double digits.
        string_stream.append(milliseconds);

        return timer_str;
    }

    // Convert a string view to character array for record names.
    static bn::array<char, 3> to_record_name(const bn::string_view view)
    {
        return {view[0], view[1], view[2]};
    }

    // Convert a record position (0 - 4) to ordinal string (1st - 5th).
    static bn::string<3> ordinal_string(int position)
    {
        if (position == 0) { return "1st"; }
        else if (position == 1) { return "2nd"; }
        else if (position == 2) { return "3rd"; }
        else if (position == 3) { return "4th"; }
        else if (position == 4) { return "5th"; }
        else { BN_ASSERT(false, "Invalid position: ", position); return "NUL"; }
    }
};