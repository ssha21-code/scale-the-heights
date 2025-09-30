#include <cmath>
#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <unordered_map>
#include <vector>
#include "ExtraHeader.h"

constexpr int WINDOW_WIDTH = 1000;
constexpr int WINDOW_HEIGHT = 800;

Texture2D textureGrass;
Texture2D texturePlayer;
Texture2D textureSky;
Texture2D texturePlatform;
Texture2D textureWalkingPlatform;
Texture2D texturePowerup;
Texture2D textureTitleScreen;
Texture2D textureStart;
Texture2D texturePlay;
Texture2D texturePause;

Sound soundSelection;
Sound soundJump;
Sound soundPickup;
Sound soundFail;
Music calmMusic;

Font font1;
Font font2;

void loadMedia() {
    textureGrass = LoadTexture("images\\grass.png");
    texturePlayer = LoadTexture("images\\player.png");
    textureSky = LoadTexture("images\\sky.png");
    texturePlatform = LoadTexture("images\\platform.png");
    textureWalkingPlatform = LoadTexture("images\\walking_platform.png");
    texturePowerup = LoadTexture("images\\powerup.png");
    textureTitleScreen = LoadTexture("images\\title_screen.png");
    textureStart = LoadTexture("images\\start.png");
    texturePlay = LoadTexture("images\\play.png");
    texturePause = LoadTexture("images\\pause.png");

    soundSelection = LoadSound("audio\\selection.wav");
    soundJump = LoadSound("audio\\jump.wav");
    soundPickup = LoadSound("audio\\pickup.wav");
    soundFail = LoadSound("audio\\fail.wav");
    calmMusic = LoadMusicStream("audio\\calm_music.wav");

    font1 = LoadFontEx("fonts\\font.ttf", 40, NULL, 0);
    font2 = LoadFontEx("fonts\\font.ttf", 30, NULL, 0);
}

class Player {
public: 
    float groundY;
    Vector2 position;
    Vector2 velocity = {8.0f, 8.0f};
    Vector2 size;
    const float gravity = 0.5;
    const float jumpStrength = -12.5f;

    bool isOnGround = true;
    bool isOnPlatform = false;

    Player(float groundY) {
        this->groundY = groundY + 50;
        this->size = {(float)texturePlayer.width, (float)texturePlayer.height};
        this->position = {100, groundY - size.y};
    }

    void draw(Vector2 playerPosition) {
        DrawTexture(texturePlayer, playerPosition.x, playerPosition.y, WHITE);
    }
    void moveLeft() {
        if (position.x > 0) {
            position.x -= velocity.x;
        }
    }
    void moveRight() {
        position.x += velocity.x;
    }
    void update() {
        if (IsKeyDown(KEY_A)) {
            moveLeft();
        }
        if (IsKeyDown(KEY_D)) {
            moveRight();
        }
        if (IsKeyPressed(KEY_SPACE) && (isOnGround || isOnPlatform)) {
            velocity.y = jumpStrength;
            PlaySound(soundJump);
        }

        velocity.y += gravity;
        position.y += velocity.y;

        if (position.y + size.y > groundY) {
            position.y = groundY - size.y;
            velocity.y = 0;
            isOnGround = true;
            isOnPlatform = false;
        } else {
            isOnGround = false;
        }
    }
};

class Platform {
public: 
    Vector2 position;
    const Vector2 size = {(float)texturePlatform.width, (float)texturePlatform.height};

    Platform(Vector2 position) {
        this->position = position;
    }

    void draw(Vector2 platformPosition) {
        DrawTexture(texturePlatform, platformPosition.x, platformPosition.y, WHITE);
    }

    void update() {

    }
};

class WalkingPlatform {
public: 
    Vector2 position;
    const Vector2 size = {(float)texturePlatform.width, (float)texturePlatform.height};

    WalkingPlatform(Vector2 position) {
        this->position = position;
    }

    void draw(Vector2 platformPosition) {
        DrawTexture(textureWalkingPlatform, platformPosition.x, platformPosition.y, WHITE);
    }

    void update() {

    }
};

class Powerup {
public: 
    Vector2 position;
    Vector2 size;
    float velocityY = 1.0f;
    int numTimesMoved = 0;

    bool shouldBeDestroyed = false;
    
    Powerup(Vector2 position) {
        this->position = position;
        this->size = {(float)texturePowerup.width, (float)texturePowerup.height};
    }

    void draw(Vector2 powerupPosition) {
        DrawTexture(texturePowerup, powerupPosition.x, powerupPosition.y, WHITE);
    }

    void update() {
        position.y += velocityY;
        numTimesMoved++;
        if (numTimesMoved % 20 == 0) {
            velocityY *= -1;
        }
    }
};

class Game {
public: 
    const float grassHeight = 150.0f;
    const float groundY = WINDOW_HEIGHT - grassHeight;
    Player player = Player(groundY);
    std::vector<Platform> platforms = {};
    std::vector<WalkingPlatform> walking_platforms = {};
    std::vector<Powerup> powerups = {};
    std::vector<int> scores = {};
    std::vector<int> yValues = {};
    std::vector<int> xValues = {};
    float cameraOffsetX = 0.0f;
    const float cameraFollowX = 200.0f;
    float cameraOffsetY = 0.0f;
    const float cameraFollowY = 200.0f;
    int playerScore = 0;

    int playerXValue;
    int playerYValue;

    int highestScore = 0;
    int highestYValue = 0;
    int highestXValue = 0;
    int numGamesPlayed = 0;

    int inGameHighestXValue = 0;
    int inGameHighestYValue = 0;

    Rectangle startButtonHitbox = {WINDOW_WIDTH - 250, 400, (float)textureStart.width, (float)textureStart.height};
    Rectangle pausePlayButtonHitbox = {WINDOW_WIDTH - 180, 48, (float)texturePause.width, (float)texturePause.height};

    bool isGameOver = false;
    bool isPaused = false;
    bool isInTitleScreen = true;

    Game() {
        createObjects();
        PlayMusicStream(calmMusic);
    }

    void createObjects() {
        float beginningX = 200.0f;
        float beginningY = groundY - 75.0f;
        float gapX = texturePlatform.width + 150.0f;
        float gapY = texturePlatform.height + 50.0f;
        bool alreadySpawnedPowerup = false;
        int numPlatforms = 20;
        int totalNumPlatforms = 999;
        
        for (int i = 0; i < totalNumPlatforms; ) {
            for (int j = 0; j < numPlatforms && i < totalNumPlatforms; i++, j++) {
                float x = beginningX + (gapX*i);
                float y = beginningY - (gapY*i);
                platforms.push_back(Platform({x, y}));
                if (i % 5 == 0) {
                    powerups.push_back(Powerup({
                        x + (texturePlatform.width - texturePowerup.width) / 2, 
                        y - texturePowerup.height - 50
                    }));
                }
            }

            if (i < totalNumPlatforms) {
                float y = beginningY - (gapY*i);

                for (int j = 0; j < numPlatforms && i < totalNumPlatforms; i++, j++) {
                    float x = beginningX + (gapX * i);
                    walking_platforms.push_back(WalkingPlatform({x, y}));
                    if (i % 5 == 0) {
                        powerups.push_back(Powerup({
                            x + (textureWalkingPlatform.width - texturePowerup.width) / 2, 
                            y - texturePowerup.height - 50
                        }));
                    }
                }
                
                beginningY += gapY * (numPlatforms - 1);
            }
        }
    }

    void draw() {
        DrawTextEx(font1, "Please Download Media from GitHub repository", {50, 50}, 40, 1, WHITE);
        if (isInTitleScreen) {
            DrawTexture(textureTitleScreen, 0, 0, WHITE);
            DrawTexture(textureStart, startButtonHitbox.x, startButtonHitbox.y, WHITE);
            if (numGamesPlayed > 0) {
                DrawTextEx(font2, TextFormat("Num. games played: %i", numGamesPlayed), {80, 500}, 30, 1, BLACK);
                DrawTextEx(font2, TextFormat("Highest X value: %i", highestXValue), {80, 540}, 30, 1, BLACK);
                DrawTextEx(font2, TextFormat("Highest Y value: %i", highestYValue), {80, 580}, 30, 1, BLACK);
                DrawTextEx(font2, TextFormat("Highest Score: %i", highestScore), {80, 620}, 30, 1, BLACK);
            }
        } else {
            DrawTexture(textureSky, 0, 0, WHITE);
    
            float grassTileWidth = textureGrass.width;
            float snappedOffsetX = floorf(-cameraOffsetX);
            float startX = -fmod(snappedOffsetX, grassTileWidth) - grassTileWidth;
            float endX = WINDOW_WIDTH + grassTileWidth;
    
            for (float x = startX; x < endX; x += grassTileWidth) {
                DrawTexture(textureGrass, x, groundY + cameraOffsetY, WHITE);
            }
            player.draw({player.position.x + cameraOffsetX, player.position.y + cameraOffsetY});  
            for (Platform &platform: platforms) {
                platform.draw({platform.position.x + cameraOffsetX, platform.position.y + cameraOffsetY});
            }
            for (WalkingPlatform &walking_platform: walking_platforms) {
                walking_platform.draw({walking_platform.position.x + cameraOffsetX, walking_platform.position.y + cameraOffsetY});
            }
            for (Powerup &powerup: powerups) {
                powerup.draw({powerup.position.x + cameraOffsetX, powerup.position.y + cameraOffsetY});
            }
            playerXValue = fabs(player.position.x) / 10;
            DrawTextEx(font2, TextFormat("X: %i", playerXValue), {50, 90}, 30, 1, BLACK);
            playerYValue = fabs((player.position.y + player.size.y) - groundY - 50) / 10;
            DrawTextEx(font2, TextFormat("Y: %i", playerYValue), {50, 120}, 30, 1, BLACK);
            DrawTextEx(font1, TextFormat("Score: %i", playerScore), {50, 50}, 40, 1, BLACK);
            if (playerXValue >= inGameHighestXValue) {
                inGameHighestXValue = playerXValue;
            }
            if (playerYValue >= inGameHighestYValue) {
                inGameHighestYValue = playerYValue;
            }
            if (isPaused) {
                DrawTexture(texturePlay, pausePlayButtonHitbox.x, pausePlayButtonHitbox.y, WHITE);
            } else {
                DrawTexture(texturePause, pausePlayButtonHitbox.x, pausePlayButtonHitbox.y, WHITE);
            }
        }
    }

    void update() {
        if (isInTitleScreen) {
            Vector2 mousePos = GetMousePosition();
            if (CheckCollisionPointRec(mousePos, startButtonHitbox) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                PlaySound(soundSelection);
                isInTitleScreen = false;
            }
        } else {
            if (!isPaused) {
                UpdateMusicStream(calmMusic);
                player.update();
                for (Platform &platform: platforms) {
                    platform.update();
                }
                for (WalkingPlatform &walking_platform: walking_platforms) {
                    walking_platform.update();
                }
                for (Powerup &powerup: powerups) {
                    powerup.update();
                }
        
                float targetCameraOffsetX = 0.0f;
                if (player.position.x > cameraFollowX) {
                    targetCameraOffsetX = cameraFollowX - player.position.x;
                }
        
                float targetCameraOffsetY = 0.0f;
                if (player.position.y < cameraFollowY) {
                    targetCameraOffsetY = cameraFollowY - player.position.y;
                }
        
                if (player.isOnGround && player.position.x > 640) {
                    isGameOver = true;
                }
                if (isGameOver) {
                    resetGame();
                }
        
                cameraOffsetX += (targetCameraOffsetX - cameraOffsetX) * 0.1f;
                cameraOffsetY += (targetCameraOffsetY - cameraOffsetY) * 0.1f;
        
                checkForCollisions();
                garbageCollect();
            }
            Vector2 mousePos = GetMousePosition();
            if (CheckCollisionPointRec(mousePos, pausePlayButtonHitbox) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                PlaySound(soundSelection);
                isPaused = !isPaused;
            }
        }
    }
    void checkForCollisions() {
        player.isOnPlatform = false;
        for (Platform &platform: platforms) {
            Collision::ResolveCollision(player.position, player.size, player.velocity, {platform.position.x, platform.position.y, platform.size.x, platform.size.y}, player.isOnPlatform);
        }
        for (WalkingPlatform &walking_platform: walking_platforms) {
            Collision::ResolveCollision(player.position, player.size, player.velocity, {walking_platform.position.x, walking_platform.position.y, walking_platform.size.x, walking_platform.size.y}, player.isOnPlatform);
        }
        for (Powerup &powerup: powerups) {
            if (CheckCollisionRecs({player.position.x, player.position.y, player.size.x, player.size.y}, {powerup.position.x, powerup.position.y, powerup.size.x, powerup.size.y})) {
                playerScore++;
                powerup.shouldBeDestroyed = true;
                PlaySound(soundPickup);
            }
        }
    }
    void garbageCollect() {
        for (int i = 0; i < powerups.size(); i++) {
            if (powerups.at(i).shouldBeDestroyed) {
                powerups.erase(powerups.begin() + i);
                i--;
            }
        }
    }
    void resetGame() {
        static double lastUpdatedTime;
        static bool shouldUpdateTime = true;
        if (shouldUpdateTime) {
            lastUpdatedTime = GetTime();
            shouldUpdateTime = false;
        }
        double currentTime = GetTime();
        if (currentTime - lastUpdatedTime >= 1.0) {
            scores.push_back(playerScore);
            xValues.push_back(inGameHighestXValue);
            yValues.push_back(inGameHighestYValue);
            setHighScores();
            playerScore = 0;
            player.position = {100.0f, groundY - player.size.y};
            platforms.clear();
            walking_platforms.clear();
            powerups.clear();
            createObjects();
            isInTitleScreen = true;
            isPaused = false;
            isGameOver = false;
            shouldUpdateTime = true;
            numGamesPlayed++;
            PlaySound(soundFail);
        }
    }
    void setHighScores() {
        for (int i: scores) {
            if (i >= highestScore) {
                highestScore = i;
            }
        }
        for (int i: xValues) {
            if (i >= highestXValue) {
                highestXValue = i;
            }
        }
        for (int i: yValues) {
            if (i >= highestYValue) {
                highestYValue = i;
            }
        }
    }
};

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Scale The Heights");
    InitAudioDevice();
    SetTargetFPS(60);

    loadMedia();
    Game game = Game();

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        game.update();
        game.draw();

        EndDrawing();
    }

    UnloadTexture(textureGrass);
    UnloadTexture(texturePlayer);
    UnloadTexture(textureSky);
    UnloadTexture(texturePlatform);
    UnloadTexture(textureWalkingPlatform);
    UnloadTexture(texturePowerup);
    UnloadTexture(textureTitleScreen);
    UnloadTexture(textureStart);
    UnloadTexture(texturePlay);
    UnloadTexture(texturePause);
    UnloadSound(soundFail);
    UnloadSound(soundJump);
    UnloadSound(soundPickup);
    UnloadSound(soundSelection);
    UnloadMusicStream(calmMusic);
    CloseWindow();
    return 0;
}