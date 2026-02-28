#include "obstacles.h"
#include "constants.h"
#include "bn_sprite_items_cactus.h"

Obstacles::Obstacles() : _spawn_timer(60)
{
}

bool Obstacles::update(bn::fixed scroll_speed)
{
    bool spawned = false;

    --_spawn_timer;
    if (_spawn_timer <= 0)
    {
        // Try to recycle an off-screen cactus first
        bool reused = false;
        for (bn::sprite_ptr &cactus : _cacti)
        {
            if (cactus.x() < constants::OFFSCREEN_LEFT)
            {
                cactus.set_position(constants::SPAWN_X, constants::GROUND_Y);
                reused = true;
                break;
            }
        }

        // If nothing to recycle, create a new one (up to the pool limit)
        if (!reused && _cacti.size() < _cacti.max_size())
        {
            _cacti.push_back(
                bn::sprite_items::cactus.create_sprite(constants::SPAWN_X, constants::GROUND_Y));
        }

        // Randomize the gap between spawns: 60–120 frames
        _spawn_timer = 60 + _random.get_int(60);
        spawned = true;
    }

    // Move all obstacles
    for (bn::sprite_ptr &cactus : _cacti)
    {
        cactus.set_x(cactus.x() + scroll_speed);
    }

    return spawned;
}

void Obstacles::reset()
{
    _spawn_timer = 30;
    for (bn::sprite_ptr &cactus : _cacti)
    {
        cactus.set_x(constants::OFFSCREEN_LEFT - 20);
    }
}
