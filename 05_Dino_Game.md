# Tutorial 5: Building a Dino Runner

This tutorial builds a simple endless runner inspired by the Chrome Dino game using Butano. We'll build it piece by piece so you understand each part before moving on.

By the end, you'll have:
- A dino that responds to jump input with gravity physics.
- Obstacles that scroll across the screen and respawn.
- AABB collision detection.
- A score counter and game-over screen with restart.

---

## 1. Assets

You need two 16×16 indexed-color BMP sprites in `graphics/`:
- `dino.bmp` — your player character.
- `cactus.bmp` — the obstacle.

Each sprite also needs a matching `.json` file so Butano knows how to import it. For a 16×16 sprite, the JSON looks like:

```json
{
    "type": "sprite"
}
```

> If your sprites are a different size, you'll need to adjust the collision constants later. We'll call those out when we get there.

---

## 2. Starting Point: Show the Dino

Before we add any game logic, let's just get the dino on screen. This confirms your sprite pipeline is working.

```cpp
#include "bn_core.h"
#include "bn_sprite_items_dino.h"

int main()
{
    bn::core::init();

    // Create the dino sprite at x=-80 (left side), y=40 (near bottom).
    // The GBA screen is 240×160, and Butano centers the origin at (0,0),
    // so valid coordinates run from -120 to 119 horizontally
    // and -80 to 79 vertically.
    bn::sprite_ptr dino = bn::sprite_items::dino.create_sprite(-80, 40);

    while(true)
    {
        bn::core::update();
    }
}
```

Build and run. You should see your dino sprite sitting near the bottom-left of the screen. If you see it, your asset pipeline works and we can start adding gameplay.

---

## 3. Adding Jump Physics

An endless runner needs a jump. We'll use simple velocity-based physics: pressing A gives the dino an upward velocity, and gravity pulls it back down every frame.

Add these includes at the top:

```cpp
#include "bn_keypad.h"
#include "bn_fixed.h"
```

Then replace the game loop with:

```cpp
// Physics state
bn::fixed y_velocity = 0;
bool grounded = true;

// Tuning constants
constexpr bn::fixed GRAVITY = 0.25;
constexpr bn::fixed JUMP_STRENGTH = -6;  // Negative = upward
constexpr bn::fixed GROUND_Y = 40;       // Must match the dino's starting y

while(true)
{
    // Jump when A is pressed (only if on the ground)
    if(grounded && bn::keypad::a_pressed())
    {
        y_velocity = JUMP_STRENGTH;
        grounded = false;
    }

    // Apply gravity every frame
    y_velocity += GRAVITY;
    dino.set_y(dino.y() + y_velocity);

    // Clamp to ground
    if(dino.y() >= GROUND_Y)
    {
        dino.set_y(GROUND_Y);
        y_velocity = 0;
        grounded = true;
    }

    bn::core::update();
}
```

**What's happening here:**
- `bn::fixed` is Butano's fixed-point number type. The GBA has no FPU, so fixed-point math is how you get "decimal" values without floating point.
- `JUMP_STRENGTH` is negative because the GBA's y-axis points **down** — negative y is up.
- `GRAVITY` is a small positive value added every frame, gradually reversing the upward velocity.
- The ground clamp prevents the dino from falling through the floor.

Build and run. Press A — the dino should jump and land smoothly.

---

## 4. Spawning Obstacles

Now we need things to dodge. We'll spawn cactus sprites off the right side of the screen and scroll them left. When a cactus goes off-screen on the left, we'll recycle it for the next spawn instead of creating a new one (the GBA has limited sprite memory).

Add these includes:

```cpp
#include "bn_vector.h"
#include "bn_random.h"
#include "bn_sprite_items_cactus.h"
```

Add these variables before the game loop:

```cpp
bn::random random;
bn::vector<bn::sprite_ptr, 16> cacti;  // Pool of up to 16 cactus sprites

int spawn_timer = 60;  // Frames until next spawn

constexpr bn::fixed SCROLL_SPEED = -2;     // Pixels per frame (negative = leftward)
constexpr bn::fixed SPAWN_X = 140;         // Just off the right edge
constexpr bn::fixed OFFSCREEN_LEFT = -140; // Past the left edge
```

Then inside the game loop (after the gravity/ground code), add:

```cpp
// --- Spawn obstacles ---
--spawn_timer;
if(spawn_timer <= 0)
{
    // Try to recycle an off-screen cactus first
    bool reused = false;
    for(bn::sprite_ptr& cactus : cacti)
    {
        if(cactus.x() < OFFSCREEN_LEFT)
        {
            cactus.set_position(SPAWN_X, GROUND_Y);
            reused = true;
            break;
        }
    }

    // If nothing to recycle, create a new one (up to the pool limit)
    if(!reused && cacti.size() < cacti.max_size())
    {
        cacti.push_back(
            bn::sprite_items::cactus.create_sprite(SPAWN_X, GROUND_Y)
        );
    }

    // Randomize the gap between spawns: 60–120 frames
    spawn_timer = 60 + random.get_int(60);
}

// --- Move obstacles ---
for(bn::sprite_ptr& cactus : cacti)
{
    cactus.set_x(cactus.x() + SCROLL_SPEED);
}
```

**Key design decisions:**
- **Object pooling with `bn::vector`:** On the GBA you can't just `new` sprites freely. We pre-allocate a fixed-size vector and recycle sprites that have scrolled off-screen. This is a pattern you'll use in almost every GBA game.
- **Randomized spawn timing:** `random.get_int(60)` returns a value in `[0, 59]`, so the gap between cacti is 60–119 frames (1–2 seconds at 60fps). This keeps the game from feeling repetitive.

Build and run. You should see cacti appearing from the right and scrolling left while you jump over them.

---

## 5. Collision Detection

The game needs to know when the dino hits a cactus. We'll use **AABB (Axis-Aligned Bounding Box)** collision — the simplest form of 2D collision detection. Two rectangles overlap if they overlap on both axes.

Add this include:

```cpp
#include "bn_math.h"  // for bn::abs
```

Add these constants (adjust if your sprites aren't 16×16):

```cpp
// Half-dimensions for collision boxes.
// For a 16×16 sprite, the box extends 8 pixels in each direction from center.
constexpr bn::fixed DINO_HALF_WIDTH = 8;
constexpr bn::fixed DINO_HALF_HEIGHT = 8;
constexpr bn::fixed CACTUS_HALF_WIDTH = 8;
constexpr bn::fixed CACTUS_HALF_HEIGHT = 8;
```

Add the collision check inside the game loop, after moving the obstacles:

```cpp
// --- Collision detection ---
bn::fixed dino_x = dino.x();
bn::fixed dino_y = dino.y();

for(const bn::sprite_ptr& cactus : cacti)
{
    // Skip cacti that are off-screen
    if(cactus.x() < OFFSCREEN_LEFT)
    {
        continue;
    }

    // AABB overlap test:
    // Two boxes overlap when the distance between centers
    // is less than the sum of their half-sizes on BOTH axes.
    bool overlap_x = bn::abs(dino_x - cactus.x()) < DINO_HALF_WIDTH + CACTUS_HALF_WIDTH;
    bool overlap_y = bn::abs(dino_y - cactus.y()) < DINO_HALF_HEIGHT + CACTUS_HALF_HEIGHT;

    if(overlap_x && overlap_y)
    {
        // Collision! We'll handle this in the next step.
    }
}
```

> **Tip:** If collisions feel unfair, shrink the half-dimensions by a pixel or two. Many games use a hitbox slightly smaller than the visual sprite to give the player some forgiveness.

---

## 6. Score, Game Over, and Restart

Now let's tie it all together with a game state machine. We need three things:
1. A score that increments every frame during play.
2. A game-over screen when you hit a cactus.
3. A way to restart by pressing START.

Add these includes:

```cpp
#include "bn_string.h"
#include "bn_sprite_text_generator.h"
#include "common_variable_8x16_sprite_font.h"
```

We'll use an enum to track the game state:

```cpp
enum class State { PLAY, GAME_OVER };
State game_state = State::PLAY;
```

Set up the text generator and score before the loop:

```cpp
bn::sprite_text_generator text_generator(common::variable_8x16_sprite_font);
text_generator.set_right_alignment();
bn::vector<bn::sprite_ptr, 16> text_sprites;

int score = 0;
```

**Why `text_sprites` is cleared every frame:** Butano's text generator creates sprite objects for each character. We store them in a vector and clear it each frame so old text doesn't linger. The sprites are destroyed when cleared from the vector, freeing VRAM.

Now wrap the gameplay code in a state check:

```cpp
while(true)
{
    text_sprites.clear();

    if(game_state == State::PLAY)
    {
        // --- Score display ---
        ++score;
        bn::string<16> score_text = "SCORE: ";
        score_text += bn::to_string<16>(score);
        text_generator.generate(115, -70, score_text, text_sprites);

        // ... all the jump, spawn, move, and collision code from above ...

        // In the collision block, trigger game over:
        if(overlap_x && overlap_y)
        {
            game_state = State::GAME_OVER;
            break;  // Stop checking other cacti
        }
    }
    else  // State::GAME_OVER
    {
        text_generator.set_center_alignment();
        text_generator.generate(0, 0, "GAME OVER", text_sprites);

        bn::string<32> score_text = "SCORE: ";
        score_text += bn::to_string<16>(score);
        text_generator.generate(0, 16, score_text, text_sprites);
        text_generator.generate(0, 32, "PRESS START", text_sprites);

        if(bn::keypad::start_pressed())
        {
            // Reset everything
            score = 0;
            spawn_timer = 30;
            y_velocity = 0;
            grounded = true;
            dino.set_position(-80, GROUND_Y);

            // Move all cacti off-screen so they can be recycled
            for(bn::sprite_ptr& cactus : cacti)
            {
                cactus.set_x(OFFSCREEN_LEFT - 20);
            }

            text_generator.set_right_alignment();
            game_state = State::PLAY;
        }
    }

    bn::core::update();
}
```

**What's happening in the reset:**
- We don't destroy the cactus sprites — we move them off-screen. The spawn logic will recycle them naturally. This avoids unnecessary allocation/deallocation.
- We restore the text alignment to right (for the score display) since we changed it to center for the game-over screen.

---

## 7. The Complete Code

Here's everything assembled together. If you've been following along, your `src/main.cpp` should look like this:

```cpp
#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_fixed.h"
#include "bn_math.h"
#include "bn_string.h"
#include "bn_vector.h"
#include "bn_random.h"
#include "bn_sprite_text_generator.h"

#include "common_variable_8x16_sprite_font.h"
#include "bn_sprite_items_dino.h"
#include "bn_sprite_items_cactus.h"

int main()
{
    bn::core::init();

    // --- Sprites ---
    bn::sprite_ptr dino = bn::sprite_items::dino.create_sprite(-80, 40);

    // --- Text ---
    bn::sprite_text_generator text_generator(common::variable_8x16_sprite_font);
    text_generator.set_right_alignment();
    bn::vector<bn::sprite_ptr, 16> text_sprites;

    // --- Obstacles ---
    bn::random random;
    bn::vector<bn::sprite_ptr, 16> cacti;

    // --- Game State ---
    enum class State { PLAY, GAME_OVER };
    State game_state = State::PLAY;

    int score = 0;
    int spawn_timer = 60;

    // --- Physics ---
    bn::fixed y_velocity = 0;
    bool grounded = true;

    // --- Constants ---
    constexpr bn::fixed GRAVITY = 0.25;
    constexpr bn::fixed JUMP_STRENGTH = -6;
    constexpr bn::fixed GROUND_Y = 40;

    constexpr bn::fixed SCROLL_SPEED = -2;
    constexpr bn::fixed SPAWN_X = 140;
    constexpr bn::fixed OFFSCREEN_LEFT = -140;

    constexpr bn::fixed DINO_HALF_WIDTH = 8;
    constexpr bn::fixed DINO_HALF_HEIGHT = 8;
    constexpr bn::fixed CACTUS_HALF_WIDTH = 8;
    constexpr bn::fixed CACTUS_HALF_HEIGHT = 8;

    while(true)
    {
        text_sprites.clear();

        if(game_state == State::PLAY)
        {
            // --- Score ---
            ++score;
            bn::string<16> score_text = "SCORE: ";
            score_text += bn::to_string<16>(score);
            text_generator.generate(115, -70, score_text, text_sprites);

            // --- Jump input ---
            if(grounded && bn::keypad::a_pressed())
            {
                y_velocity = JUMP_STRENGTH;
                grounded = false;
            }

            // --- Gravity and vertical movement ---
            y_velocity += GRAVITY;
            dino.set_y(dino.y() + y_velocity);

            if(dino.y() >= GROUND_Y)
            {
                dino.set_y(GROUND_Y);
                y_velocity = 0;
                grounded = true;
            }

            // --- Spawn obstacles ---
            --spawn_timer;
            if(spawn_timer <= 0)
            {
                bool reused = false;
                for(bn::sprite_ptr& cactus : cacti)
                {
                    if(cactus.x() < OFFSCREEN_LEFT)
                    {
                        cactus.set_position(SPAWN_X, GROUND_Y);
                        reused = true;
                        break;
                    }
                }

                if(!reused && cacti.size() < cacti.max_size())
                {
                    cacti.push_back(bn::sprite_items::cactus.create_sprite(SPAWN_X, GROUND_Y));
                }

                spawn_timer = 60 + random.get_int(60);
            }

            // --- Move obstacles ---
            for(bn::sprite_ptr& cactus : cacti)
            {
                cactus.set_x(cactus.x() + SCROLL_SPEED);
            }

            // --- Collision ---
            bn::fixed dino_x = dino.x();
            bn::fixed dino_y = dino.y();

            for(const bn::sprite_ptr& cactus : cacti)
            {
                if(cactus.x() < OFFSCREEN_LEFT)
                {
                    continue;
                }

                if(bn::abs(dino_x - cactus.x()) < DINO_HALF_WIDTH + CACTUS_HALF_WIDTH &&
                   bn::abs(dino_y - cactus.y()) < DINO_HALF_HEIGHT + CACTUS_HALF_HEIGHT)
                {
                    game_state = State::GAME_OVER;
                    break;
                }
            }
        }
        else
        {
            // --- Game Over Screen ---
            text_generator.set_center_alignment();
            text_generator.generate(0, 0, "GAME OVER", text_sprites);

            bn::string<32> score_text = "SCORE: ";
            score_text += bn::to_string<16>(score);
            text_generator.generate(0, 16, score_text, text_sprites);
            text_generator.generate(0, 32, "PRESS START", text_sprites);

            if(bn::keypad::start_pressed())
            {
                score = 0;
                spawn_timer = 30;
                y_velocity = 0;
                grounded = true;

                dino.set_position(-80, GROUND_Y);

                for(bn::sprite_ptr& cactus : cacti)
                {
                    cactus.set_x(OFFSCREEN_LEFT - 20);
                }

                text_generator.set_right_alignment();
                game_state = State::PLAY;
            }
        }

        bn::core::update();
    }
}
```

---

## 8. Ideas to Explore Next

Now that you have a working game, here are some things to try:

- **Speed ramp:** Increase `SCROLL_SPEED` gradually as the score goes up to make the game harder over time.
- **Animated sprites:** Use Butano's sprite animation system to give the dino a running animation.
- **Ground line:** Use a background or a row of sprites to draw a visible ground.
- **High score:** Track the best score across restarts (or use SRAM to persist it across power cycles).
- **Sound effects:** Play a sound on jump and on collision using Butano's audio system.
