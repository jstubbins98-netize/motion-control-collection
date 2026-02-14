#include "rhythm_dance_game.h"

struct DancePose {
    std::string name;
    float leftArmX, leftArmY;
    float rightArmX, rightArmY;
    float duration;
};

void runRhythmDance(SDL_Renderer* renderer, PoseDetector& poseDetector, cv::VideoCapture& cap,
                    bool useCamera, cv::Mat& currentFrame, bool& returnToMenu, Mix_Music* bgMusic) {
    Mix_Music* danceMusic = Mix_LoadMUS("audio and sound effects/rhythm_dance.mp3");
    if (danceMusic) {
        Mix_PlayMusic(danceMusic, -1);
        Mix_VolumeMusic(MIX_MAX_VOLUME / 2);
    }
    
    std::vector<DancePose> poses = {
        {"ARMS UP", 0.3f, 0.2f, 0.7f, 0.2f, 2.0f},
        {"ARMS DOWN", 0.3f, 0.8f, 0.7f, 0.8f, 2.0f},
        {"LEFT HIGH", 0.2f, 0.2f, 0.7f, 0.6f, 2.0f},
        {"RIGHT HIGH", 0.3f, 0.6f, 0.8f, 0.2f, 2.0f},
        {"T POSE", 0.1f, 0.5f, 0.9f, 0.5f, 2.0f},
        {"HANDS CENTER", 0.45f, 0.4f, 0.55f, 0.4f, 2.0f},
    };
    
    int score = 0;
    int combo = 0;
    int currentPoseIdx = 0;
    float poseTimer = 0;
    float beatTimer = 0;
    bool running = true;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> poseDist(0, poses.size() - 1);

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
        }

        poseTimer += 0.016f;
        beatTimer += 0.016f;
        
        DancePose& targetPose = poses[currentPoseIdx];
        
        float leftDist = std::sqrt(
            std::pow(poseDetector.leftHandX - targetPose.leftArmX, 2) +
            std::pow(poseDetector.leftHandY - targetPose.leftArmY, 2)
        );
        float rightDist = std::sqrt(
            std::pow(poseDetector.rightHandX - targetPose.rightArmX, 2) +
            std::pow(poseDetector.rightHandY - targetPose.rightArmY, 2)
        );
        
        bool poseMatched = (leftDist < 0.2f && rightDist < 0.2f);
        
        if (poseTimer > targetPose.duration) {
            if (poseMatched) {
                combo++;
                score += 100 * combo;
            } else {
                combo = 0;
            }
            currentPoseIdx = poseDist(gen);
            poseTimer = 0;
        }

        float beatPulse = std::sin(beatTimer * 4) * 0.5f + 0.5f;

        SDL_SetRenderDrawColor(renderer, (Uint8)(30 + beatPulse * 20), 20, (Uint8)(60 + beatPulse * 40), 255);
        SDL_Rect bg = {0, 0, GAME_WIDTH, GAME_HEIGHT};
        SDL_RenderFillRect(renderer, &bg);

        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 100);
        int silhouetteX = GAME_WIDTH / 4;
        int silhouetteY = GAME_HEIGHT / 2;
        drawFilledCircle(renderer, silhouetteX, silhouetteY - 100, 40);
        SDL_Rect body = {silhouetteX - 30, silhouetteY - 60, 60, 120};
        SDL_RenderFillRect(renderer, &body);
        
        int leftTargetX = (int)(targetPose.leftArmX * GAME_WIDTH / 2);
        int leftTargetY = (int)(targetPose.leftArmY * GAME_HEIGHT);
        int rightTargetX = (int)(targetPose.rightArmX * GAME_WIDTH / 2);
        int rightTargetY = (int)(targetPose.rightArmY * GAME_HEIGHT);
        
        SDL_SetRenderDrawColor(renderer, PINK.r, PINK.g, PINK.b, 200);
        SDL_RenderDrawLine(renderer, silhouetteX, silhouetteY - 50, leftTargetX, leftTargetY);
        SDL_RenderDrawLine(renderer, silhouetteX, silhouetteY - 50, rightTargetX, rightTargetY);
        drawFilledCircle(renderer, leftTargetX, leftTargetY, 20);
        drawFilledCircle(renderer, rightTargetX, rightTargetY, 20);

        float progress = poseTimer / targetPose.duration;
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_Rect progressBg = {50, GAME_HEIGHT - 40, GAME_WIDTH - 100, 20};
        SDL_RenderFillRect(renderer, &progressBg);
        
        if (poseMatched) {
            SDL_SetRenderDrawColor(renderer, GREEN.r, GREEN.g, GREEN.b, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, ORANGE.r, ORANGE.g, ORANGE.b, 255);
        }
        SDL_Rect progressBar = {50, GAME_HEIGHT - 40, (int)((GAME_WIDTH - 100) * (1 - progress)), 20};
        SDL_RenderFillRect(renderer, &progressBar);

        SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
        drawNumber(renderer, score, 20, 20, 30);
        
        if (combo > 1) {
            SDL_SetRenderDrawColor(renderer, YELLOW.r, YELLOW.g, YELLOW.b, YELLOW.a);
            drawNumber(renderer, combo, GAME_WIDTH/2 - 20, 20, 25);
        }

        if (poseMatched) {
            SDL_SetRenderDrawColor(renderer, GREEN.r, GREEN.g, GREEN.b, 255);
            SDL_Rect matchBox = {GAME_WIDTH/2 - 60, 80, 120, 40};
            SDL_RenderFillRect(renderer, &matchBox);
        }

        drawCameraFeed(renderer, currentFrame, useCamera);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    
    if (danceMusic) {
        Mix_FreeMusic(danceMusic);
    }
    if (bgMusic) {
        Mix_PlayMusic(bgMusic, -1);
        Mix_VolumeMusic(MIX_MAX_VOLUME / 3);
    }
}
