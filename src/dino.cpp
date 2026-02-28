#include "dino.h"
#include "constants.h"
#include "bn_keypad.h"
#include "bn_sprite_items_dino.h"

Dino::Dino() : _sprite(bn::sprite_items::dino.create_sprite(-100, constants::GROUND_Y)),
               _y_velocity(0),
               _grounded(true)
{
    _sprite.set_visible(false);
}

void Dino::update()
{
    _y_velocity += constants::GRAVITY;
    _sprite.set_y(_sprite.y() + _y_velocity);

    if (_sprite.y() >= constants::GROUND_Y)
    {
        _sprite.set_y(constants::GROUND_Y);
        _y_velocity = 0;
        _grounded = true;
    }
}

void Dino::handle_input()
{
    if (_grounded && (bn::keypad::a_pressed() || bn::keypad::b_pressed() || bn::keypad::up_pressed()))
    {
        jump();
    }

    if (bn::keypad::left_held() && _sprite.x() > -100)
    {
        _sprite.set_x(_sprite.x() - constants::DINO_SPEED);
    }
    if (bn::keypad::right_held() && _sprite.x() < 100)
    {
        _sprite.set_x(_sprite.x() + constants::DINO_SPEED);
    }
}

void Dino::jump()
{
    _y_velocity = constants::JUMP_STRENGTH;
    _grounded = false;
}

void Dino::reset()
{
    _y_velocity = 0;
    _grounded = true;
    _sprite.set_position(-80, constants::GROUND_Y);
}
