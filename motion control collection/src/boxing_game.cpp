#include "boxing_game.h"

struct BoxingTarget {
    float x, y;
    int radius;
    float lifetime;
    float maxLifetime;
    bool active;
    Color color;
};

void runBoxing(SDL_Renderer* renderer, PoseDetector& poseDetector, cv::VideoCapture& cap,
               bool useCamera, cv::Mat& currentFrame, bool& returnToMenu, Mix_Music* menuMusic) {
    Mix_Music* boxingMusic = Mix_LoadMUS("audio and sound effects/boxing_background.mp3");
    
    if (menuMusic) {
        Mix_HaltMusic();
    }
    
    if (boxingMusic) {
        Mix_PlayMusic(boxingMusic, -1);
        Mix_VolumeMusic(MIX_MAX_VOLUME / 3);
    }
    
    std::vector<BoxingTarget> targets;
    int score = 0;
    int combo = 0;
    float spawnTimer = 0;
    bool running = true;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> xDist(100, GAME_WIDTH - 100);
    std::uniform_real_distribution<> yDist(100, GAME_HEIGHT - 100);
    std::uniform_int_distribution<> colorDist(0, 2);
    
    float leftHandScreenX = GAME_WIDTH * 0.3f;
    float leftHandScreenY = GAME_HEIGHT * 0.5f;
    float rightHandScreenX = GAME_WIDTH * 0.7f;
    float rightHandScreenY = GAME_HEIGHT * 0.5f;

    while (running && !returnToMenu) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) { running = false; returnToMenu = false; }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) returnToMenu = true;
                if (event.key.keysym.sym == SDLK_q) returnToMenu = true;
            }
        }

        if (useCamera && cap.read(currentFrame)) {
            cv::flip(currentFrame, currentFrame, 1);
            poseDetector.detectBothHands(currentFrame);
            leftHandScreenX = poseDetector.leftHandX * GAME_WIDTH;
            leftHandScreenY = poseDetector.leftHandY * GAME_HEIGHT;
            rightHandScreenX = poseDetector.rightHandX * GAME_WIDTH;
            rightHandScreenY = poseDetector.rightHandY * GAME_HEIGHT;
        } else {
            int mx, my;
            SDL_GetMouseState(&mx, &my);
            rightHandScreenX = mx;
            rightHandScreenY = my;
            
            const Uint8* keys = SDL_GetKeyboardState(NULL);
            if (keys[SDL_SCANCODE_W]) leftHandScreenY -= 10;
            if (keys[SDL_SCANCODE_S]) leftHandScreenY += 10;
            if (keys[SDL_SCANCODE_A]) leftHandScreenX -= 10;
            if (keys[SDL_SCANCODE_D]) leftHandScreenX += 10;
            leftHandScreenX = std::max(0.0f, std::min((float)GAME_WIDTH, leftHandScreenX));
            leftHandScreenY = std::max(0.0f, std::min((float)GAME_HEIGHT, leftHandScreenY));
        }

        spawnTimer += 0.016f;
        if (spawnTimer > 1.5f && targets.size() < 5) {
            BoxingTarget t;
            t.x = xDist(gen);
            t.y = yDist(gen);
            t.radius = 40 + (std::rand() % 20);
            t.maxLifetime = 3.0f;
            t.lifetime = t.maxLifetime;
            t.active = true;
            Color colors[] = {RED, ORANGE, YELLOW};
            t.color = colors[colorDist(gen)];
            targets.push_back(t);
            spawnTimer = 0;
        }

        for (auto& t : targets) {
            if (!t.active) continue;
            t.lifetime -= 0.016f;
            if (t.lifetime <= 0) { t.active = false; combo = 0; continue; }

            float dxL = leftHandScreenX - t.x;
            float dyL = leftHandScreenY - t.y;
            float dxR = rightHandScreenX - t.x;
            float dyR = rightHandScreenY - t.y;
            
            if (std::sqrt(dxL*dxL + dyL*dyL) < t.radius || std::sqrt(dxR*dxR + dyR*dyR) < t.radius) {
                t.active = false;
                combo++;
                score += 100 * combo;
            }
        }

        targets.erase(std::remove_if(targets.begin(), targets.end(), 
            [](const BoxingTarget& t) { return !t.active; }), targets.end());

        SDL_SetRenderDrawColor(renderer, 30, 30, 50, 255);
        SDL_Rect bg = {0, 0, GAME_WIDTH, GAME_HEIGHT};
        SDL_RenderFillRect(renderer, &bg);

        for (auto& t : targets) {
            if (!t.active) continue;
            float alpha = t.lifetime / t.maxLifetime;
            SDL_SetRenderDrawColor(renderer, t.color.r, t.color.g, t.color.b, (Uint8)(alpha * 255));
            drawFilledCircle(renderer, (int)t.x, (int)t.y, t.radius);
            SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, 255);
            for (int i = 0; i < 3; i++) {
                for (int a = 0; a < 360; a++) {
                    int px = (int)t.x + (int)((t.radius - i) * cos(a * 3.14159 / 180));
                    int py = (int)t.y + (int)((t.radius - i) * sin(a * 3.14159 / 180));
                    SDL_RenderDrawPoint(renderer, px, py);
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, BLUE.r, BLUE.g, BLUE.b, BLUE.a);
        drawFilledCircle(renderer, (int)leftHandScreenX, (int)leftHandScreenY, 25);
        SDL_SetRenderDrawColor(renderer, RED.r, RED.g, RED.b, RED.a);
        drawFilledCircle(renderer, (int)rightHandScreenX, (int)rightHandScreenY, 25);

        SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
        drawNumber(renderer, score, 20, 20, 30);
        
        if (combo > 1) {
            SDL_SetRenderDrawColor(renderer, YELLOW.r, YELLOW.g, YELLOW.b, YELLOW.a);
            drawNumber(renderer, combo, GAME_WIDTH - 100, 20, 25);
        }

        drawCameraFeed(renderer, currentFrame, useCamera);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    
    Mix_HaltMusic();
    if (boxingMusic) Mix_FreeMusic(boxingMusic);
    
    if (menuMusic) {
        Mix_PlayMusic(menuMusic, -1);
        Mix_VolumeMusic(MIX_MAX_VOLUME / 3);
    }
}
