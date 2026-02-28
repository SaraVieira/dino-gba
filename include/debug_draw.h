#ifndef DEBUG_DRAW_H
#define DEBUG_DRAW_H

#include "bn_sprite_ptr.h"
#include "bn_vector.h"
#include "dino.h"
#include "obstacles.h"

class DebugDraw
{
public:
    DebugDraw();

    void toggle() { _enabled = !_enabled; }
    void update(const Dino& dino, const Obstacles& obstacles);

private:
    bn::vector<bn::sprite_ptr, 64> _sprites;
    bool _enabled;
};

#endif
