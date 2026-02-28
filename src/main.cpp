#include "bn_core.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_text_generator.h"
#include "bn_vector.h"
#include "bn_fixed.h"
#include "bn_keypad.h"
#include "bn_vector.h"
#include "bn_random.h"
#include "bn_math.h"
#include "bn_string.h"
#include "common_variable_8x16_sprite_font.h"

#include "bn_sprite_items_dino.h"
#include "bn_sprite_items_cactus.h"
#include "bn_sprite_items_debug_dot.h"

int main()
{
    bn::core::init();
    enum class State
    {
        MENU,
        PLAY,
        PAUSE,
        GAME_OVER
    };
    int score = 0;
    State game_state = State::MENU;
    bn::sprite_text_generator text_generator(common::variable_8x16_sprite_font);
    text_generator.set_right_alignment();
    bn::vector<bn::sprite_ptr, 16> text_sprites;

    constexpr bn::fixed HALF_SPRITE_SIZE = 16;
    constexpr bn::fixed Y_START = 60; // Must match the dino's starting y

    // Physics state
    bn::fixed y_velocity = 0;
    bool grounded = true;

    bn::fixed dino_speed = 2; // Horizontal movement speed

    bn::random random;
    bn::vector<bn::sprite_ptr, 16> cacti; // Pool of up to 16 cactus sprites
    bn::sprite_ptr dino = bn::sprite_items::dino.create_sprite(-100, Y_START);
    dino.set_visible(false);
    int spawn_timer = 60; // Frames until next spawn
    constexpr bn::fixed DINO_HALF_WIDTH = HALF_SPRITE_SIZE - 4;
    constexpr bn::fixed DINO_HALF_HEIGHT = HALF_SPRITE_SIZE - 2;
    constexpr bn::fixed CACTUS_HALF_WIDTH = HALF_SPRITE_SIZE / 2; // Cacti are skinnier than the dino
    constexpr bn::fixed CACTUS_HALF_HEIGHT = HALF_SPRITE_SIZE - 4;
    constexpr bn::fixed SCROLL_SPEED = -2;     // Pixels per frame (negative = leftward)
    constexpr bn::fixed SPAWN_X = 140;         // Just off the right edge
    constexpr bn::fixed OFFSCREEN_LEFT = -140; // Past the left edge

    // Tuning constants
    constexpr bn::fixed GRAVITY = 0.25;
    constexpr bn::fixed JUMP_STRENGTH = -6; // Negative = upward
    constexpr bn::fixed GROUND_Y = Y_START; // Must match the dino's starting y

    // Debug hitbox visualization (toggle with SELECT)
    bool show_hitboxes = true;
    bn::vector<bn::sprite_ptr, 64> debug_sprites;

    while (true)
    {
        text_sprites.clear();
        debug_sprites.clear();

        if (game_state == State::PLAY)
        {

            if (bn::keypad::start_pressed())
            {
                game_state = State::PAUSE;
            }

            text_generator.set_right_alignment();
            dino.set_visible(true);
            // Jump when A is pressed (only if on the ground)
            if (grounded && bn::keypad::a_pressed())
            {
                y_velocity = JUMP_STRENGTH;
                grounded = false;
            }

            // Apply gravity every frame
            y_velocity += GRAVITY;
            dino.set_y(dino.y() + y_velocity);

            // Clamp to ground
            if (dino.y() >= GROUND_Y)
            {
                dino.set_y(GROUND_Y);
                y_velocity = 0;
                grounded = true;
            }

            if (bn::keypad::left_held() && dino.x() > -100)
            {
                dino.set_x(dino.x() - dino_speed);
            }
            if (bn::keypad::right_held() && dino.x() < 100)
            {
                dino.set_x(dino.x() + dino_speed);
            }

            // --- Spawn obstacles ---
            --spawn_timer;
            if (spawn_timer <= 0)
            {
                ++score;
                // Try to recycle an off-screen cactus first
                bool reused = false;
                for (bn::sprite_ptr &cactus : cacti)
                {
                    if (cactus.x() < OFFSCREEN_LEFT)
                    {
                        cactus.set_position(SPAWN_X, GROUND_Y);
                        reused = true;
                        break;
                    }
                }

                // If nothing to recycle, create a new one (up to the pool limit)
                if (!reused && cacti.size() < cacti.max_size())
                {
                    cacti.push_back(
                        bn::sprite_items::cactus.create_sprite(SPAWN_X, GROUND_Y));
                }

                // Randomize the gap between spawns: 60–120 frames
                spawn_timer = 60 + random.get_int(60);
            }

            // --- Move obstacles ---
            for (bn::sprite_ptr &cactus : cacti)
            {
                cactus.set_x(cactus.x() + SCROLL_SPEED);
            }

            bn::fixed dino_x = dino.x();
            bn::fixed dino_y = dino.y();

            for (const bn::sprite_ptr &cactus : cacti)
            {
                // Skip cacti that are off-screen
                if (cactus.x() < OFFSCREEN_LEFT)
                {
                    continue;
                }

                // AABB overlap test:
                // Two boxes overlap when the distance between centers
                // is less than the sum of their half-sizes on BOTH axes.
                bool overlap_x = bn::abs(dino_x - cactus.x()) < DINO_HALF_WIDTH + CACTUS_HALF_WIDTH;
                bool overlap_y = bn::abs(dino_y - cactus.y()) < DINO_HALF_HEIGHT + CACTUS_HALF_HEIGHT;

                if (overlap_x && overlap_y)
                {
                    game_state = State::GAME_OVER;
                    break; // Stop checking other cacti
                }
            }

            // --- Debug hitbox display ---
            if (show_hitboxes)
            {
                // Dino hitbox corners
                bn::fixed dx = dino.x();
                bn::fixed dy = dino.y();
                debug_sprites.push_back(bn::sprite_items::debug_dot.create_sprite(dx - DINO_HALF_WIDTH, dy - DINO_HALF_HEIGHT));
                debug_sprites.push_back(bn::sprite_items::debug_dot.create_sprite(dx + DINO_HALF_WIDTH, dy - DINO_HALF_HEIGHT));
                debug_sprites.push_back(bn::sprite_items::debug_dot.create_sprite(dx - DINO_HALF_WIDTH, dy + DINO_HALF_HEIGHT));
                debug_sprites.push_back(bn::sprite_items::debug_dot.create_sprite(dx + DINO_HALF_WIDTH, dy + DINO_HALF_HEIGHT));

                // Cactus hitbox corners
                for (const bn::sprite_ptr &cactus : cacti)
                {
                    if (cactus.x() < OFFSCREEN_LEFT || debug_sprites.size() + 4 > debug_sprites.max_size())
                    {
                        continue;
                    }
                    bn::fixed cx = cactus.x();
                    bn::fixed cy = cactus.y();
                    debug_sprites.push_back(bn::sprite_items::debug_dot.create_sprite(cx - CACTUS_HALF_WIDTH, cy - CACTUS_HALF_HEIGHT));
                    debug_sprites.push_back(bn::sprite_items::debug_dot.create_sprite(cx + CACTUS_HALF_WIDTH, cy - CACTUS_HALF_HEIGHT));
                    debug_sprites.push_back(bn::sprite_items::debug_dot.create_sprite(cx - CACTUS_HALF_WIDTH, cy + CACTUS_HALF_HEIGHT));
                    debug_sprites.push_back(bn::sprite_items::debug_dot.create_sprite(cx + CACTUS_HALF_WIDTH, cy + CACTUS_HALF_HEIGHT));
                }
            }

            // --- Score display ---

            bn::string<16> score_text = "SCORE: ";
            score_text += bn::to_string<16>(score);
            text_generator.generate(115, -70, score_text, text_sprites);
        }
        else if (game_state == State::GAME_OVER)
        {
            text_generator.set_center_alignment();
            text_generator.generate(0, 0, "GAME OVER", text_sprites);

            bn::string<32> score_text = "SCORE: ";
            score_text += bn::to_string<16>(score);
            text_generator.generate(0, 16, score_text, text_sprites);
            text_generator.generate(0, 32, "PRESS START", text_sprites);

            if (bn::keypad::start_pressed())
            {
                // Reset everything
                score = 0;
                spawn_timer = 30;
                y_velocity = 0;
                grounded = true;
                dino.set_position(-80, GROUND_Y);

                // Move all cacti off-screen so they can be recycled
                for (bn::sprite_ptr &cactus : cacti)
                {
                    cactus.set_x(OFFSCREEN_LEFT - 20);
                }

                text_generator.set_right_alignment();
                game_state = State::PLAY;
            }
        }
        else if (game_state == State::MENU)
        {
            text_generator.set_center_alignment();
            text_generator.generate(0, 16, "Dino Game", text_sprites);
            text_generator.generate(0, 32, "PRESS START", text_sprites);

            if (bn::keypad::start_pressed())
            {

                game_state = State::PLAY;
            }
        }
        else if (game_state == State::PAUSE)
        {
            text_generator.set_center_alignment();
            text_generator.generate(0, 16, "PAUSED", text_sprites);
            text_generator.generate(0, 32, "PRESS START", text_sprites);

            if (bn::keypad::start_pressed())
            {
                game_state = State::PLAY;
            }
        }
        bn::core::update();
    }
}
