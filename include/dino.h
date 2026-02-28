#ifndef DINO_H
#define DINO_H

#include "bn_sprite_ptr.h"
#include "bn_fixed.h"

class Dino
{
public:
    Dino();

    void update();
    void handle_input();
    void jump();

    bn::fixed x() const { return _sprite.x(); }
    bn::fixed y() const { return _sprite.y(); }
    void set_position(bn::fixed x, bn::fixed y) { _sprite.set_position(x, y); }
    void set_visible(bool visible) { _sprite.set_visible(visible); }
    const bn::sprite_ptr& sprite() const { return _sprite; }

    void reset();

private:
    bn::sprite_ptr _sprite;
    bn::fixed _y_velocity;
    bool _grounded;
};

#endif
