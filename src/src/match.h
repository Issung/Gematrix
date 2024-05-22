#ifndef MATCH
#define MATCH

#include "bn_vector.h"

class match_position
{
public:
    int row;
    int col;

    match_position(int new_row, int new_col) : row(new_row), col(new_col)
    {

    }
};

class match
{
public:
    static constexpr int MAX_LENGTH = 5;
    bn::vector<match_position, MAX_LENGTH> positions;

    match(bn::vector<match_position, MAX_LENGTH> new_positions) : positions(new_positions)
    {

    }
};

#endif