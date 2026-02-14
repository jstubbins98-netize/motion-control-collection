#include "common.h"
#include "menu.h"
#include "pickleball_game.h"
#include "boxing_game.h"
#include "tennis_game.h"
#include "archery_game.h"
#include "rhythm_dance_game.h"

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL could not initialize: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer could not initialize: " << Mix_GetError() << std::endl;
    }

    Mix_Music* bgMusic = Mix_LoadMUS("audio and sound effects/background.mp3");
    if (bgMusic) {
        Mix_PlayMusic(bgMusic, -1);
        Mix_VolumeMusic(MIX_MAX_VOLUME / 3);
    }

    SDL_Window* window = SDL_CreateWindow(
        "Motion Games - Use Your Body!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, GAME_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "Window could not be created: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created: " << SDL_GetError() << std::endl;
        return 1;
    }

    cv::VideoCapture cap;
#ifdef __APPLE__
    cap.open(0, cv::CAP_AVFOUNDATION);
#else
    cap.open(0);
#endif
    bool useCamera = cap.isOpened();
    if (useCamera) {
        std::cout << "Camera detected. Warming up..." << std::endl;
        cv::Mat warmup;
        for (int i = 0; i < 30; i++) {
            cap.read(warmup);
            SDL_Delay(30);
        }
        std::cout << "Camera ready!" << std::endl;
    } else {
        std::cout << "Camera not available. Using keyboard/mouse controls." << std::endl;
    }

    PoseDetector poseDetector;
    if (useCamera) {
        poseDetector.init();
    }

    GameState state = MENU;
    int selectedGame = 0;
    bool running = true;
    bool showingInfo = false;
    cv::Mat currentFrame;

    while (running) {
        SDL_Event event;
        
        if (state == MENU) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) running = false;
                else if (event.type == SDL_KEYDOWN) {
                    if (showingInfo) {
                        showingInfo = false;
                    } else {
                        switch (event.key.keysym.sym) {
                            case SDLK_UP: case SDLK_w: selectedGame = (selectedGame + 4) % 5; break;
                            case SDLK_DOWN: case SDLK_s: selectedGame = (selectedGame + 1) % 5; break;
                            case SDLK_RETURN: case SDLK_SPACE:
                                state = (GameState)(selectedGame + 1);
                                break;
                            case SDLK_1: selectedGame = 0; break;
                            case SDLK_2: selectedGame = 1; break;
                            case SDLK_3: selectedGame = 2; break;
                            case SDLK_4: selectedGame = 3; break;
                            case SDLK_5: selectedGame = 4; break;
                            case SDLK_i: case SDLK_h: showingInfo = true; break;
                            case SDLK_q: running = false; break;
                        }
                    }
                }
            }

            if (useCamera && cap.read(currentFrame)) {
                cv::flip(currentFrame, currentFrame, 1);
            }

            drawMenu(renderer, selectedGame);
            if (showingInfo) {
                drawInfoPopup(renderer, selectedGame);
            }
            drawCameraFeed(renderer, currentFrame, useCamera);
            SDL_RenderPresent(renderer);
            SDL_Delay(16);
        } else {
            bool returnToMenu = false;
            
            switch (state) {
                case PICKLEBALL:
                    runPickleball(renderer, poseDetector, cap, useCamera, currentFrame, returnToMenu);
                    break;
                case BOXING:
                    runBoxing(renderer, poseDetector, cap, useCamera, currentFrame, returnToMenu, bgMusic);
                    break;
                case TENNIS:
                    runTennis(renderer, poseDetector, cap, useCamera, currentFrame, returnToMenu);
                    break;
                case ARCHERY:
                    runArchery(renderer, poseDetector, cap, useCamera, currentFrame, returnToMenu, bgMusic);
                    break;
                case RHYTHM_DANCE:
                    runRhythmDance(renderer, poseDetector, cap, useCamera, currentFrame, returnToMenu, bgMusic);
                    break;
                default:
                    break;
            }
            
            if (returnToMenu) {
                state = MENU;
            } else {
                running = false;
            }
        }
    }

    if (bgMusic) Mix_FreeMusic(bgMusic);
    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
