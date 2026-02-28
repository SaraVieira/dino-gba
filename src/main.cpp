#include "bn_core.h"
#include "bn_sprite_text_generator.h"
#include "bn_vector.h"
#include "bn_keypad.h"
#include "bn_string.h"
#include "bn_math.h"
#include "common_variable_8x16_sprite_font.h"

#include "constants.h"
#include "dino.h"
#include "obstacles.h"
#include "debug_draw.h"

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
    bn::fixed scroll_speed = -2;
    int score = 0;
    State game_state = State::MENU;
    bn::sprite_text_generator text_generator(common::variable_8x16_sprite_font);
    text_generator.set_right_alignment();
    bn::vector<bn::sprite_ptr, 16> text_sprites;

    Dino dino;
    Obstacles obstacles;
    DebugDraw debug_draw;

    while (true)
    {
        text_sprites.clear();

        if (game_state == State::PLAY)
        {
            if (bn::keypad::start_pressed())
            {
                game_state = State::PAUSE;
            }

            text_generator.set_right_alignment();
            dino.set_visible(true);
            dino.handle_input();
            dino.update();

            if (obstacles.update(scroll_speed))
            {
                ++score;
                scroll_speed = -2 - (bn::fixed(0.005) * score);
            }

            // Collision detection
            bn::fixed dino_x = dino.x();
            bn::fixed dino_y = dino.y();

            for (const bn::sprite_ptr &cactus : obstacles.cacti())
            {
                if (cactus.x() < constants::OFFSCREEN_LEFT)
                {
                    continue;
                }

                bool overlap_x = bn::abs(dino_x - cactus.x()) < constants::DINO_HALF_WIDTH + constants::CACTUS_HALF_WIDTH;
                bool overlap_y = bn::abs(dino_y - cactus.y()) < constants::DINO_HALF_HEIGHT + constants::CACTUS_HALF_HEIGHT;

                if (overlap_x && overlap_y)
                {
                    game_state = State::GAME_OVER;
                    break;
                }
            }

            debug_draw.update(dino, obstacles);

            // Score display
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
                score = 0;
                dino.reset();
                obstacles.reset();
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
