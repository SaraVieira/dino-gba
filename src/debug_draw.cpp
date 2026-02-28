#include "debug_draw.h"
#include "constants.h"
#include "bn_sprite_items_debug_dot.h"

DebugDraw::DebugDraw() : _enabled(false)
{
}

void DebugDraw::update(const Dino &dino, const Obstacles &obstacles)
{
    _sprites.clear();

    if (!_enabled)
    {
        return;
    }

    // Dino hitbox corners
    bn::fixed dx = dino.x();
    bn::fixed dy = dino.y();
    _sprites.push_back(bn::sprite_items::debug_dot.create_sprite(dx - constants::DINO_HALF_WIDTH, dy - constants::DINO_HALF_HEIGHT));
    _sprites.push_back(bn::sprite_items::debug_dot.create_sprite(dx + constants::DINO_HALF_WIDTH, dy - constants::DINO_HALF_HEIGHT));
    _sprites.push_back(bn::sprite_items::debug_dot.create_sprite(dx - constants::DINO_HALF_WIDTH, dy + constants::DINO_HALF_HEIGHT));
    _sprites.push_back(bn::sprite_items::debug_dot.create_sprite(dx + constants::DINO_HALF_WIDTH, dy + constants::DINO_HALF_HEIGHT));

    // Cactus hitbox corners
    for (const bn::sprite_ptr &cactus : obstacles.cacti())
    {
        if (cactus.x() < constants::OFFSCREEN_LEFT || _sprites.size() + 4 > _sprites.max_size())
        {
            continue;
        }
        bn::fixed cx = cactus.x();
        bn::fixed cy = cactus.y();
        _sprites.push_back(bn::sprite_items::debug_dot.create_sprite(cx - constants::CACTUS_HALF_WIDTH, cy - constants::CACTUS_HALF_HEIGHT));
        _sprites.push_back(bn::sprite_items::debug_dot.create_sprite(cx + constants::CACTUS_HALF_WIDTH, cy - constants::CACTUS_HALF_HEIGHT));
        _sprites.push_back(bn::sprite_items::debug_dot.create_sprite(cx - constants::CACTUS_HALF_WIDTH, cy + constants::CACTUS_HALF_HEIGHT));
        _sprites.push_back(bn::sprite_items::debug_dot.create_sprite(cx + constants::CACTUS_HALF_WIDTH, cy + constants::CACTUS_HALF_HEIGHT));
    }
}
