#ifndef PLAYER
#define PLAYER

#include "bn_keypad.h"
#include "bn_sprites.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_items_caveman.h"

class Player {
private:
    int x = 0;
    int y = 0;

public:
    // Constructor
    Player()
    {
    }

    // Destructor
    ~Player()
    {
    }

    bn::sprite_ptr update()
    {
        if (bn::keypad::left_held())
        {
            x -= 1;
        }
        if (bn::keypad::right_held())
        {
            x += 1;
        }
        if (bn::keypad::up_held())
        {
            y -= 1;
        }
        if (bn::keypad::down_held())
        {
            y += 1;
        }

        bn::sprite_ptr caveman = bn::sprite_items::caveman.create_sprite(x, y);
        caveman.set_bg_priority(2);
        caveman.set_z_order(1);

        return caveman;
    }
};

#endif