#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "bn_fixed.h"

namespace constants
{
    constexpr bn::fixed HALF_SPRITE_SIZE = 16;
    constexpr bn::fixed GROUND_Y = 60;

    // Physics
    constexpr bn::fixed GRAVITY = 0.25;
    constexpr bn::fixed JUMP_STRENGTH = -6;

    // Player
    constexpr bn::fixed DINO_SPEED = 2;
    constexpr bn::fixed DINO_HALF_WIDTH = HALF_SPRITE_SIZE - 4;
    constexpr bn::fixed DINO_HALF_HEIGHT = HALF_SPRITE_SIZE - 2;

    // Obstacles
    constexpr bn::fixed CACTUS_HALF_WIDTH = HALF_SPRITE_SIZE / 2;
    constexpr bn::fixed CACTUS_HALF_HEIGHT = HALF_SPRITE_SIZE - 4;
    constexpr bn::fixed SPAWN_X = 140;
    constexpr bn::fixed OFFSCREEN_LEFT = -140;
}

#endif
