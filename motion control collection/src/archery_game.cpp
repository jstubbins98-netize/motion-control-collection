#include "archery_game.h"

struct Arrow {
    float x, y;
    float angle;
    float power;
    float speedX, speedY;
    bool flying = false;
    bool drawing = false;
};

struct ArcheryTarget {
    float x, y;
    int rings[5] = {60, 48, 36, 24, 12};
};

void runArchery(SDL_Renderer* renderer, PoseDetector& poseDetector, cv::VideoCapture& cap,
                bool useCamera, cv::Mat& currentFrame, bool& returnToMenu, Mix_Music* menuMusic) {
    Mix_Music* archeryMusic = Mix_LoadMUS("audio and sound effects/archery_background.mp3");
    Mix_Chunk* arrowSound = Mix_LoadWAV("audio and sound effects/arrow_shoot.mp3");
    
    if (menuMusic) {
        Mix_HaltMusic();
    }
    
    if (archeryMusic) {
        Mix_PlayMusic(archeryMusic, -1);
        Mix_VolumeMusic(MIX_MAX_VOLUME / 3);
    }
    
    Arrow arrow;
    ArcheryTarget target;
    int score = 0;
    int arrowsLeft = 10;
    bool running = true;
    
    target.x = GAME_WIDTH - 150;
    target.y = GAME_HEIGHT / 2.0f;
    
    float aimY = GAME_HEIGHT / 2.0f;
    float drawAmount = 0;
    bool isDrawing = false;
    float prevArmY = 0.5f;

    while (running && !returnToMenu) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) { running = false; returnToMenu = false; }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) returnToMenu = true;
                if (event.key.keysym.sym == SDLK_q) returnToMenu = true;
                if (event.key.keysym.sym == SDLK_SPACE && !arrow.flying && arrowsLeft > 0) {
                    isDrawing = true;
                }
            }
            else if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym == SDLK_SPACE && isDrawing && !arrow.flying) {
                    arrow.x = 100;
                    arrow.y = aimY;
                    arrow.speedX = drawAmount * 20;
                    arrow.speedY = 0;
                    arrow.flying = true;
                    arrowsLeft--;
                    isDrawing = false;
                    drawAmount = 0;
                    if (arrowSound) {
                        Mix_PlayChannel(-1, arrowSound, 0);
                    }
                }
            }
        }

        if (useCamera && cap.read(currentFrame)) {
            cv::flip(currentFrame, currentFrame, 1);
            float armY = poseDetector.detectArmPosition(currentFrame);
            aimY = armY * GAME_HEIGHT;
            
            float armDelta = std::abs(armY - prevArmY);
            if (armDelta > 0.02f && !arrow.flying) {
                isDrawing = true;
                drawAmount = std::min(1.0f, drawAmount + armDelta * 2);
            } else if (isDrawing && drawAmount > 0.3f && armDelta < 0.01f && !arrow.flying && arrowsLeft > 0) {
                arrow.x = 100;
                arrow.y = aimY;
                arrow.speedX = drawAmount * 20;
                arrow.speedY = 0;
                arrow.flying = true;
                arrowsLeft--;
                isDrawing = false;
                drawAmount = 0;
                if (arrowSound) {
                    Mix_PlayChannel(-1, arrowSound, 0);
                }
            }
            prevArmY = armY;
        } else {
            const Uint8* keys = SDL_GetKeyboardState(NULL);
            if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) aimY -= 5;
            if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) aimY += 5;
            if (isDrawing) drawAmount = std::min(1.0f, drawAmount + 0.02f);
        }
        aimY = std::max(50.0f, std::min((float)GAME_HEIGHT - 50, aimY));

        if (arrow.flying) {
            arrow.x += arrow.speedX;
            arrow.speedY += 0.3f;
            arrow.y += arrow.speedY;

            float dx = arrow.x - target.x;
            float dy = arrow.y - target.y;
            float dist = std::sqrt(dx*dx + dy*dy);
            
            if (dist < target.rings[0]) {
                arrow.flying = false;
                if (dist < target.rings[4]) score += 100;
                else if (dist < target.rings[3]) score += 80;
                else if (dist < target.rings[2]) score += 60;
                else if (dist < target.rings[1]) score += 40;
                else score += 20;
            }
            
            if (arrow.x > GAME_WIDTH || arrow.y > GAME_HEIGHT || arrow.y < 0) {
                arrow.flying = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255);
        SDL_Rect sky = {0, 0, GAME_WIDTH, GAME_HEIGHT};
        SDL_RenderFillRect(renderer, &sky);

        SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
        SDL_Rect ground = {0, GAME_HEIGHT - 100, GAME_WIDTH, 100};
        SDL_RenderFillRect(renderer, &ground);

        Color ringColors[] = {WHITE, BLACK, BLUE, RED, YELLOW};
        for (int i = 0; i < 5; i++) {
            SDL_SetRenderDrawColor(renderer, ringColors[i].r, ringColors[i].g, ringColors[i].b, 255);
            drawFilledCircle(renderer, (int)target.x, (int)target.y, target.rings[i]);
        }

        if (!arrow.flying) {
            SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255);
            SDL_Rect bow = {70, (int)aimY - 50, 10, 100};
            SDL_RenderFillRect(renderer, &bow);

            if (isDrawing) {
                int pullBack = (int)(drawAmount * 50);
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
                SDL_RenderDrawLine(renderer, 75, (int)aimY - 50, 100 - pullBack, (int)aimY);
                SDL_RenderDrawLine(renderer, 75, (int)aimY + 50, 100 - pullBack, (int)aimY);
            }
        }

        if (arrow.flying) {
            SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255);
            float angle = std::atan2(arrow.speedY, arrow.speedX);
            int len = 40;
            int x2 = (int)(arrow.x - len * std::cos(angle));
            int y2 = (int)(arrow.y - len * std::sin(angle));
            SDL_RenderDrawLine(renderer, (int)arrow.x, (int)arrow.y, x2, y2);
            SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
            drawFilledCircle(renderer, (int)arrow.x, (int)arrow.y, 4);
        }

        SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
        drawNumber(renderer, score, 20, 20, 30);
        drawNumber(renderer, arrowsLeft, GAME_WIDTH - 100, 20, 25);

        drawCameraFeed(renderer, currentFrame, useCamera);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    
    Mix_HaltMusic();
    if (archeryMusic) Mix_FreeMusic(archeryMusic);
    if (arrowSound) Mix_FreeChunk(arrowSound);
    
    if (menuMusic) {
        Mix_PlayMusic(menuMusic, -1);
        Mix_VolumeMusic(MIX_MAX_VOLUME / 3);
    }
}
