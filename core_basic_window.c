#include <stdio.h>
#include "raylib.h"
#include "math.h"
#include "raymath.h"

#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

int gameScreenWidth = 640;
int gameScreenHeight = 480;
float jumpSpeed = 350.0f;
int floorY = 250;
float gravity = 450.0f;

typedef struct Player {
    Vector2 position;
    float speed;
    bool canJump;
} Player;

typedef struct EnvItem {
    Rectangle rect;
    int blocking;
    Color color;
} EnvItem;


void UpdatePlayer(Player *player, Sound sound, EnvItem *envItems, int envItemsLength, float delta);

int main()
{
    // initialize game
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(gameScreenWidth, gameScreenHeight, "Yobby Run");
    InitAudioDevice();
    SetRandomSeed(12345);
    SetTargetFPS(60);
    Image icon = LoadImage("../resources/icon.ico");
    SetWindowIcon(icon);

    // screen
    SetWindowMinSize(320, 240);
    RenderTexture2D target = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);


    // initialize player sprite
    int animFrames = 0;
    Image imageAnimation = LoadImageAnim("../resources/YobbyRun.gif", &animFrames);
    Texture2D textureAnim = LoadTextureFromImage(imageAnimation);

    // initialize music and sound
    Music music;
    Sound sound;
    // https://pixabay.com/music/video-games-cruising-down-8bit-lane-159615/
    music = LoadMusicStream("../resources/cruising-down-8bit-lane.mp3");
    // https://pixabay.com/sound-effects/cartoon-jump-6462/
    sound = LoadSound("../resources/jump.mp3");
    SetMusicVolume(music, 0.1f);
    SetSoundVolume(sound, 0.1f);
    PlayMusicStream(music);

    // frame data
    unsigned int nextFrameDataOffset = 0;
    int currentAnimFrame = 0;
    int frameDelay = 8;
    int frameCounter = 0;

    // initialize player data
    Player player = {0};
    player.position = (Vector2) {gameScreenWidth / 3, gameScreenHeight / 1.5};
    player.speed = 0;
    player.canJump = false;

    EnvItem envItems[] = {
        {{ 0, gameScreenHeight / 1.5, 1000, 200 }, 1, RAYWHITE }
    };

    int envElementLength = sizeof(envItems)/sizeof(envItems[0]);
    // Main game loop
    while (!WindowShouldClose())
    {

        float scale = MIN((float)GetScreenWidth()/gameScreenWidth, (float)GetScreenHeight()/gameScreenHeight);

        float deltaTime = GetFrameTime();
        UpdateMusicStream(music);

        if(GetMusicTimePlayed(music) >= 39.0f) {
            SeekMusicStream(music, 0.69f);
        }
        // animation stuff
        frameCounter++;
        if (frameCounter >= frameDelay) {

            currentAnimFrame++;
            if (currentAnimFrame >= animFrames) {
                currentAnimFrame = 0;
            }

            nextFrameDataOffset = imageAnimation.width * imageAnimation.height * 4 * currentAnimFrame;

            UpdateTexture(textureAnim, ((unsigned char *) imageAnimation.data) + nextFrameDataOffset);

            frameCounter = 0;
        }

        UpdatePlayer(&player, sound, envItems, envElementLength, deltaTime);

        BeginDrawing();
            BeginTextureMode(target);
            ClearBackground(RAYWHITE);
            DrawFPS(5, 5);

            for (int i = 0; i < envElementLength; i++) {
                DrawRectangleRec(envItems[i].rect, envItems[i].color);
            }

            // DrawCircleV(player.position, 5.0f, GOLD);
            DrawTexture(textureAnim, player.position.x - textureAnim.width / 2, player.position.y - textureAnim.height / 2, WHITE);
            DrawText("Yeaa", gameScreenWidth - 220, gameScreenHeight - 20, 10, GRAY);
        EndTextureMode();
        DrawTexturePro(target.texture, (Rectangle){ 0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height },
                           (Rectangle){ (GetScreenWidth() - ((float)gameScreenWidth*scale))*0.5f, (GetScreenHeight() - ((float)gameScreenHeight*scale))*0.5f,
                           (float)gameScreenWidth*scale, (float)gameScreenHeight*scale }, (Vector2){ 0, 0 }, 0.0f, WHITE);
        EndDrawing();

    }

    // unload sound & textures
    StopMusicStream(music);
    UnloadSound(sound);
    UnloadImage(imageAnimation);
    UnloadTexture(textureAnim);
    UnloadRenderTexture(target);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}

void UpdatePlayer(Player *player, Sound sound, EnvItem *envItems, int envItemsLength, float delta) {
    // if (IsKeyDown(KEY_LEFT)) player->position.x -= 200.0f*delta;
    // if (IsKeyDown(KEY_RIGHT)) player->position.x += 200.0f*delta;
    if(IsKeyDown(KEY_SPACE) && player->canJump) {
        PlaySound(sound);
        player->speed = -jumpSpeed;
        player->canJump = false;
    }

    // collision detection with items
    bool hitObstacle = false;
    for(int i = 0; i < envItemsLength; i++) {
        EnvItem *ei = envItems + i;
        Vector2 *p = &(player->position);

        if(ei->blocking &&
            ei->rect.x <= p->x &&
            ei->rect.x + ei->rect.width >= p->x &&
            ei->rect.y >= p->y &&
            ei->rect.y <= p->y + player->speed*delta) {
            hitObstacle = true;
            player->speed = 0.0f;
            p->y = ei->rect.y;
            break;
        }
    }

    // gravity
    if(!hitObstacle) {
        player->position.y += player->speed*delta;
        player->speed += gravity*delta;
        player->canJump = false;
    } else player->canJump = true;
}