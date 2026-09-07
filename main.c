#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "raylib.h"
#include <math.h>


#define PLAYER_SIZE         20.0f
#define PLAYER_SPEED        6.0f
#define PLAYER_MAX_SHOOTS   20

#define MAX_ASTEROIDS       20
#define ASTEROID_SPEED      2.0f

#define MAX_PARTICLES 60

typedef struct Particle {
    Vector2 position;
    Vector2 velocity;
    float lifetime;
    float maxLifetime;
    bool active;
} Particle;


typedef struct Player {
    Vector2 position;
    Vector2 velocity;
    float acceleration;
    float rotation;
    Color color;
    float thrustAngle;
    Vector3 collider;
    int health;
    bool invincible;
} Player;

typedef struct Shoot {
    Vector2 position;
    Vector2 velocity;
    float radius;
    float rotation;
    int lifeSpawn;
    bool active;
    Color color;
} Shoot;

typedef struct Asteroid {
    Vector2 position;
    Vector2 velocity;
    float radius;
    int stage;
    bool active;
    float rotation;
    float rotSpeed;
    int sides;
} Asteroid;



static const int screenWidth = 800;
static const int screenHeight = 800;

static Player player = { 0 };
static Shoot shoot[PLAYER_MAX_SHOOTS] = { 0 };
static Asteroid asteroids[MAX_ASTEROIDS] = { 0 };
static int score = 0;
static bool gameOver = false;
static Particle particles[MAX_PARTICLES] = { 0 };

float timer = 0.0f;
float invincibilityTimer = 1.5f;


float AsteroidRadius(int stage) {
    if (stage == 3) return 50.0f;
    if (stage == 2) return 28.0f;
    return 14.0f;
}

void SpawnParticles(Vector2 pos, int count) {

    int spawned = 0;

    for (int i = 0; i < MAX_PARTICLES && spawned < count; i++) {
        if (!particles[i].active) {
            float angle = GetRandomValue(0, 360) * DEG2RAD;
            float speed = GetRandomValue(2, 8);
            particles[i].position = pos;
            particles[i].velocity = (Vector2){ cosf(angle) * speed, sinf(angle) * speed};
            particles[i].lifetime = 0.0f;
            particles[i].maxLifetime = GetRandomValue(30, 80) * 0.01f;
            particles[i].active = true;
            spawned++;
        }
    }   
}

void SpawnAsteroid(Vector2 pos, int stage){
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!asteroids[i].active) {
            if (pos.x < 0) {
                int edge = GetRandomValue(0,3);
                if (edge == 0) pos = (Vector2){GetRandomValue(0, screenWidth), -50};
                else if (edge == 1) pos = (Vector2){screenWidth + 50, GetRandomValue(0, screenHeight)};
                else if (edge == 2) pos = (Vector2){GetRandomValue(0, screenWidth), screenHeight + 50};
                else                pos = (Vector2){-50, GetRandomValue(0, screenHeight)};
            }

            float angle = GetRandomValue(0, 360) * DEG2RAD;
            float speed = ASTEROID_SPEED + GetRandomValue(0,10) * 0.1f;

            asteroids[i].position = pos;
            asteroids[i].velocity = (Vector2){ cosf(angle) * speed, sinf(angle) * speed};
            asteroids[i].radius = AsteroidRadius(stage);
            asteroids[i].stage = stage;
            asteroids[i].active = true;
            asteroids[i].rotation = GetRandomValue(0, 359);
            asteroids[i].rotSpeed = GetRandomValue(1, 4) * (GetRandomValue(0,1) ? 1 : -1);
            asteroids[i].sides = GetRandomValue(6,8);
            break;
        }
    }
}

void ResetPlayer(void) {
    player.position = (Vector2){screenWidth/2, screenHeight/2};
    player.velocity = (Vector2){0, 0};
    player.acceleration = 0;
    player.rotation = 0;
    player.thrustAngle = 0;
    player.collider = (Vector3){player.position.x, player.position.y, 12};
    player.color = WHITE;
    player.health -= 1;
    player.invincible = true;
    timer = 0.0f;
}

void ResetGame(void) {
    score = 0;
    gameOver = false;

    player.position = (Vector2){screenWidth/2, screenHeight/2};
    player.velocity = (Vector2){0, 0};
    player.acceleration = 0;
    player.rotation = 0;
    player.thrustAngle = 0;
    player.collider = (Vector3){player.position.x, player.position.y, 12};
    player.color = WHITE;
    player.health = 3;
    player.invincible = true;
    timer = 0.0f;

    if(player.invincible) {
        timer += GetFrameTime();
        if(timer >= invincibilityTimer) {
            player.invincible = false;
            timer = 0.0f;
         }
    }


    for ( int i = 0; i < PLAYER_MAX_SHOOTS; i++)
    {
        shoot[i].position = (Vector2){0, 0};
        shoot[i].velocity = (Vector2){0, 0};
        shoot[i].radius = 2;
        shoot[i].active = false;
        shoot[i].lifeSpawn = 0;
        shoot[i].color = WHITE;
    }

    for (int i = 0; i < MAX_ASTEROIDS; i++)
    {
        asteroids[i].active = false;

    }

    int spawned = 0;
    while (spawned < 6) {
        Vector2 pos = (Vector2){-1, -1};
        SpawnAsteroid(pos, 3);
        spawned++;
    }
}

int main(void)
{
    srand(time(NULL));
    SetRandomSeed(time(NULL));

    InitWindow(screenWidth, screenHeight, "Asteroids");
    SetTargetFPS(60);


    InitAudioDevice();

    Sound fire = LoadSound("fire.wav");
    Sound explosion = LoadSound("bangSmall.wav");   

    

    ResetGame();



    while (!WindowShouldClose())
    {

        for(int i = 0; i < MAX_PARTICLES; i++) {
            if (!particles[i].active) continue;
            particles[i].position.x += particles[i].velocity.x;
            particles[i].position.y += particles[i].velocity.y;
            particles[i].velocity.x *= 0.95f;
            particles[i].velocity.y *= 0.95f;
            particles[i].lifetime += GetFrameTime();
            if (particles[i].lifetime >= particles[i].maxLifetime) particles[i].active = false;
        }

        if(player.invincible) {
            timer += GetFrameTime();
            if(timer >= invincibilityTimer) {
                player.invincible = false;
                timer = 0.0f;
            }
        }

        if(gameOver) {
            if (IsKeyPressed(KEY_ENTER)) ResetGame();
        } else {
            //PLAYER MOVEMENT
            player.velocity.x = cos(player.thrustAngle*DEG2RAD) * PLAYER_SPEED;
            player.velocity.y = sin(player.thrustAngle*DEG2RAD) * PLAYER_SPEED;

            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
            {
                player.rotation -= 4;
                
            };
            if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) 
            {
                player.rotation += 4;
                
            }
            

            if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
            {
                if(player.acceleration < 1) player.acceleration += 0.02f;
                player.thrustAngle = player.rotation;
                player.thrustAngle = player.rotation;
            }
            else
            {
                if (player.acceleration > 0) player.acceleration -= 0.009f;
                if (player.acceleration < 0) player.acceleration = 0;
            }

            player.position.x += (player.velocity.x*player.acceleration);
            player.position.y += (player.velocity.y*player.acceleration);

            player.collider.x = player.position.x;
            player.collider.y = player.position.y;

            if (player.position.x < -10) player.position.x = screenWidth + 10;
            if (player.position.x > screenWidth + 10) player.position.x = -10;
            if (player.position.y < -10) player.position.y = screenHeight + 10;
            if (player.position.y > screenHeight + 10) player.position.y = -10;


            //SHOOT LOGIC
            if (IsKeyPressed(KEY_SPACE))
            {
                for (int i = 0; i < PLAYER_MAX_SHOOTS; i++)
                {
                    if (!shoot[i].active)
                    {
                        shoot[i].position = (Vector2){player.position.x, player.position.y};
                        shoot[i].active = true;
                        shoot[i].velocity.x = 1.5*cos(player.rotation*DEG2RAD)*PLAYER_SPEED;
                        shoot[i].velocity.y = -1.5*sin(player.rotation*DEG2RAD)*PLAYER_SPEED;
                        shoot[i].rotation = player.rotation;
                        break;
                    }
                }
                PlaySound(fire);
            }

            for (int i = 0; i < PLAYER_MAX_SHOOTS; i++)
            {
                if (shoot[i].active) shoot[i].lifeSpawn++;
            }

            for (int i = 0; i < PLAYER_MAX_SHOOTS; i++){
                if (shoot[i].active)
                {

                    shoot[i].position.x += shoot[i].velocity.x;
                    shoot[i].position.y -= shoot[i].velocity.y;

                    if (shoot[i].position.x < -10) shoot[i].position.x = screenWidth + 10;
                    if (shoot[i].position.x > screenWidth + 10) shoot[i].position.x = -10;
                    if (shoot[i].position.y < -10) shoot[i].position.y = screenHeight + 10;
                    if (shoot[i].position.y > screenHeight + 10) shoot[i].position.y = -10;

                    if (shoot[i].lifeSpawn >= 100)
                    {
                        shoot[i].position = (Vector2){0, 0};
                        shoot[i].velocity = (Vector2){0, 0};
                        shoot[i].lifeSpawn = 0;
                        shoot[i].active = false;
                    }
                }
            }

            for (int i = 0; i < MAX_ASTEROIDS; i++) {
                if (!asteroids[i].active) continue;

                asteroids[i].position.x += asteroids[i].velocity.x;
                asteroids[i].position.y += asteroids[i].velocity.y;
                asteroids[i].rotation += asteroids[i].rotSpeed;

                if (asteroids[i].position.x < -asteroids[i].radius) asteroids[i].position.x = screenWidth + asteroids[i].radius;
                if (asteroids[i].position.x > screenWidth + asteroids[i].radius) asteroids[i].position.x = -asteroids[i].radius;
                if (asteroids[i].position.y < -asteroids[i].radius) asteroids[i].position.y = screenHeight + asteroids[i].radius;
                if (asteroids[i].position.y > screenHeight + asteroids[i].radius) asteroids[i].position.y = -asteroids[i].radius;
                
            }

            for (int i = 0; i < PLAYER_MAX_SHOOTS; i++) {
                if (!shoot[i].active) continue;
                for (int j = 0; j < MAX_ASTEROIDS; j++){
                    if (!asteroids[j].active) continue;
                    float dx = shoot[i].position.x - asteroids[j].position.x;
                    float dy = shoot[i].position.y - asteroids[j].position.y;
                    float dist = sqrtf(dx*dx + dy*dy);

                    if (dist < shoot[i].radius + asteroids[j].radius) {

                        shoot[i].active = false;
                        shoot[i].lifeSpawn = 0;

                        if (asteroids[j].stage == 3)
                        {
                            score += 20;
                            PlaySound(explosion);
                            SpawnParticles(asteroids[j].position, 15);
                        }
                        else if (asteroids[j].stage == 2)
                        {
                            score += 50;
                            PlaySound(explosion);
                            SpawnParticles(asteroids[j].position, 10);
                        } 
                        else 
                        {
                            score += 100;
                            PlaySound(explosion);
                            SpawnParticles(asteroids[j].position, 5);
                        }
                        Vector2 hitPos = asteroids[j].position;
                        int stage = asteroids[j].stage;
                        asteroids[j].active = false;

                        if (stage > 1) {
                            SpawnAsteroid(hitPos, stage - 1);
                            SpawnAsteroid(hitPos, stage - 1);
                        } else {
                            Vector2 edge = (Vector2){-1, -1};
                            SpawnAsteroid(edge, 3);
                        }
                        break;
                    }
                }
            }

            for (int i = 0; i < MAX_ASTEROIDS; i++) {
                if (!asteroids[i].active) continue;

                float dx = player.collider.x - asteroids[i].position.x;
                float dy = player.collider.y - asteroids[i].position.y;
                float dist = sqrtf(dx*dx + dy*dy);

                if (dist < player.collider.z + asteroids[i].radius && player.invincible == false) {
                    SpawnParticles(player.position, 15);
                    ResetPlayer();
                    PlaySound(explosion);

                    if (player.health < 1){
                        gameOver = true;
                    }
                    break;
                }
            }
        }


        BeginDrawing();
        ClearBackground(BLACK);

        if (gameOver) {
            const char *msg = "GAME OVER";
            const char *msg2 = "Press ENTER to restart";
            DrawText(msg, screenWidth/2 - MeasureText(msg, 60)/2, screenHeight/2-60, 60, RAYWHITE);
            DrawText(msg2, screenWidth/2 - MeasureText(msg2, 24)/2, screenHeight/2 + 20, 24, RAYWHITE);
            DrawText( TextFormat("Score: %d", score), screenWidth/2 - 60, screenHeight/2 + 60, 28, RAYWHITE);
        } else {

            for (int i = 0; i < MAX_PARTICLES; i++) {
                if (!particles[i].active) continue;
                float alpha = 1.0f - (particles[i].lifetime / particles[i].maxLifetime);
                Color c = Fade(WHITE, alpha);
                DrawCircleV(particles[i].position, 2.0f, c);
            }
            
            for (int i = 0; i < PLAYER_MAX_SHOOTS; i++)
                {
                    if (shoot[i].active) DrawCircleV(shoot[i].position, shoot[i].radius, shoot[i].color);
                }
            

            Color drawColor = player.color;
            if(player.invincible){
                float progress = timer / invincibilityTimer;
                float blinkSpeed = 5.0f + progress * 20.0f;
                drawColor = (sinf(timer * blinkSpeed) > 0) ? WHITE: BLANK;
            }
            DrawPolyLines((Vector2){player.position.x, player.position.y}, 3, PLAYER_SIZE, player.rotation, drawColor);

            for (int i = 0; i < MAX_ASTEROIDS; i++)
            {
                if (asteroids[i].active)
                {
                    DrawPolyLines(asteroids[i].position, asteroids[i].sides, asteroids[i].radius, asteroids[i].rotation, GRAY);
                }
            }

            DrawText(TextFormat("SCORE: %d", score), 10, 10, 24, WHITE);

            for(int i = 1; i < player.health + 1; i++)
            {
                DrawPolyLines((Vector2){screenWidth - i*40, 30}, 3, PLAYER_SIZE, -90, WHITE);
            }
        }
    
    EndDrawing();
    }

    UnloadSound(fire);
    UnloadSound(explosion);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}


