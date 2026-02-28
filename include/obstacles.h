#ifndef OBSTACLES_H
#define OBSTACLES_H

#include "bn_sprite_ptr.h"
#include "bn_fixed.h"
#include "bn_vector.h"
#include "bn_random.h"

class Obstacles
{
public:
    Obstacles();

    // Returns true if a new cactus was spawned (for scoring)
    bool update(bn::fixed scroll_speed);

    const bn::vector<bn::sprite_ptr, 16>& cacti() const { return _cacti; }

    void reset();

private:
    bn::vector<bn::sprite_ptr, 16> _cacti;
    bn::random _random;
    int _spawn_timer;
};

#endif
