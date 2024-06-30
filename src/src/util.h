#pragma once

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
};