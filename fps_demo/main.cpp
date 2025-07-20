#include <SDL.h>
#include <cmath>
#include <cstdint>

// Screen dimensions
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

// Map dimensions
const int MAP_W = 16;
const int MAP_H = 16;

// Simple map layout (1 = wall, 0 = empty)
const char worldMap[MAP_H][MAP_W+1] = {
    "1111111111111111",
    "1000000000000001",
    "1011110011110101",
    "1001000000000101",
    "1001111111100101",
    "1001000000100101",
    "1001011100100101",
    "1001000100100101",
    "1001000100100101",
    "1001000111100101",
    "1001000000000001",
    "1011111111111101",
    "1000000000000001",
    "1000000000000001",
    "1000000000000001",
    "1111111111111111"
};

struct Player {
    double x = 8.0;
    double y = 8.0;
    double dir = 0.0; // radians
} player;

void drawVerticalLine(SDL_Surface* surface, int x, int y0, int y1, uint32_t color) {
    if (x < 0 || x >= surface->w) return;
    if (y0 < 0) y0 = 0;
    if (y1 >= surface->h) y1 = surface->h - 1;
    uint32_t* pixels = (uint32_t*)surface->pixels + y0 * surface->w + x;
    for (int y = y0; y <= y1; ++y) {
        *pixels = color;
        pixels += surface->w;
    }
}

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("FPS Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window) {
        SDL_Quit();
        return 1;
    }

    SDL_Surface* surface = SDL_GetWindowSurface(window);

    bool running = true;
    const double moveSpeed = 3.0; // units per second
    const double rotSpeed = 2.0;  // radians per second
    uint32_t lastTicks = SDL_GetTicks();

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
        }

        const Uint8* keys = SDL_GetKeyboardState(NULL);
        uint32_t currentTicks = SDL_GetTicks();
        double delta = (currentTicks - lastTicks) / 1000.0;
        lastTicks = currentTicks;

        if (keys[SDL_SCANCODE_ESCAPE]) running = false;
        if (keys[SDL_SCANCODE_LEFT]) player.dir -= rotSpeed * delta;
        if (keys[SDL_SCANCODE_RIGHT]) player.dir += rotSpeed * delta;

        double dx = cos(player.dir) * moveSpeed * delta;
        double dy = sin(player.dir) * moveSpeed * delta;
        if (keys[SDL_SCANCODE_UP]) {
            int mx = int(player.x + dx);
            int my = int(player.y + dy);
            if (worldMap[my][int(player.x)] == '0') player.y += dy;
            if (worldMap[int(player.y)][mx] == '0') player.x += dx;
        }
        if (keys[SDL_SCANCODE_DOWN]) {
            int mx = int(player.x - dx);
            int my = int(player.y - dy);
            if (worldMap[my][int(player.x)] == '0') player.y -= dy;
            if (worldMap[int(player.y)][mx] == '0') player.x -= dx;
        }

        // Draw frame via ray casting
        SDL_LockSurface(surface);
        uint32_t* pixels = (uint32_t*)surface->pixels;
        std::fill(pixels, pixels + surface->w * surface->h, 0x202020FF);

        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            double cameraX = 2.0 * x / SCREEN_WIDTH - 1.0; // -1..1
            double rayDir = player.dir + cameraX * (M_PI / 4.0); // 45 deg FOV
            double rayX = cos(rayDir);
            double rayY = sin(rayDir);

            double posX = player.x;
            double posY = player.y;

            double sideDistX;
            double sideDistY;

            double deltaDistX = (rayX == 0) ? 1e30 : std::fabs(1.0 / rayX);
            double deltaDistY = (rayY == 0) ? 1e30 : std::fabs(1.0 / rayY);
            double perpWallDist;

            int stepX;
            int stepY;
            int mapX = int(posX);
            int mapY = int(posY);

            if (rayX < 0) {
                stepX = -1;
                sideDistX = (posX - mapX) * deltaDistX;
            } else {
                stepX = 1;
                sideDistX = (mapX + 1.0 - posX) * deltaDistX;
            }
            if (rayY < 0) {
                stepY = -1;
                sideDistY = (posY - mapY) * deltaDistY;
            } else {
                stepY = 1;
                sideDistY = (mapY + 1.0 - posY) * deltaDistY;
            }

            int side = 0;
            while (true) {
                if (sideDistX < sideDistY) {
                    sideDistX += deltaDistX;
                    mapX += stepX;
                    side = 0;
                } else {
                    sideDistY += deltaDistY;
                    mapY += stepY;
                    side = 1;
                }
                if (worldMap[mapY][mapX] != '0') break;
            }

            if (side == 0)
                perpWallDist = (sideDistX - deltaDistX);
            else
                perpWallDist = (sideDistY - deltaDistY);

            int lineHeight = (int)(SCREEN_HEIGHT / perpWallDist);
            int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
            int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
            uint32_t color = side ? 0xFFAAAAFF : 0xFFFFFFFF;
            drawVerticalLine(surface, x, drawStart, drawEnd, color);
        }

        SDL_UnlockSurface(surface);
        SDL_UpdateWindowSurface(window);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
