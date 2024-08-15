#pragma once

#define LEVELS 3

// Constants for gamemode levels, lower is easier/faster.
class levels
{
public:
    // Score to reach.
    static constexpr unsigned int sprint[LEVELS] = {
        1000,
        2500,
        5000,
    };

    // Time measured in frames (60 fps).
    static constexpr unsigned int timeattack[LEVELS] = {
        60 * 60,   // 1 Minute
        60 * 150,  // 150 seconds = 2.5 Minutes
        60 * 300,  // 300 seconds = 5 Minutes
    };
};