#pragma once

#include "gem_type.h"
#include "bn_vector.h"

class match_position
{
public:
    int row;
    int col;

    match_position(
        int _row,
        int _col
    ) : row(_row),
        col(_col)
    {

    }
};

class match
{
public:
    // Maximum length of a match.
    static constexpr int MAX_LENGTH = 6;
    gem_type type;
    bn::vector<match_position, MAX_LENGTH> positions;

    match(
        gem_type _type,
        bn::vector<match_position, MAX_LENGTH> _positions
    ) : type(_type),
        positions(_positions)
    {

    }
};