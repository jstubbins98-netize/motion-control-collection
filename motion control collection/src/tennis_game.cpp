#include "tennis_game.h"

struct TennisBall {
    float x, y;
    float speedX, speedY;
    int radius = 15;
    bool active = false;
    bool goingRight = true;
};

void runTennis(SDL_Renderer* renderer, PoseDetector& poseDetector, cv::VideoCapture& cap,
               bool useCamera, cv::Mat& currentFrame, bool& returnToMenu) {
    TennisBall ball;
    int playerScore = 0, aiScore = 0;
    float playerY = GAME_HEIGHT / 2.0f;
    float aiY = GAME_HEIGHT / 2.0f;
    bool running = true;
    
    std::random_device rd;
    std::mt19937 gen(rd());

    auto resetBall = [&](bool toPlayer) {
        ball.x = toPlayer ? GAME_WIDTH - 100 : 100;
        ball.y = GAME_HEIGHT / 2.0f;
        ball.speedX = toPlayer ? -8.0f : 8.0f;
        ball.speedY = ((std::rand() % 100) / 100.0f - 0.5f) * 6.0f;
        ball.active = true;
        ball.goingRight = !toPlayer;
    };
    
    resetBall(true);

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
            float armY = poseDetector.detectArmPosition(currentFrame);
            playerY = armY * GAME_HEIGHT;
        } else {
            const Uint8* keys = SDL_GetKeyboardState(NULL);
            if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) playerY -= 8;
            if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) playerY += 8;
        }
        playerY = std::max(50.0f, std::min((float)GAME_HEIGHT - 50, playerY));

        if (ball.goingRight) {
            aiY += (ball.y - aiY) * 0.05f;
        }
        aiY = std::max(50.0f, std::min((float)GAME_HEIGHT - 50, aiY));

        ball.x += ball.speedX;
        ball.y += ball.speedY;

        if (ball.y < 30 || ball.y > GAME_HEIGHT - 30) ball.speedY *= -1;

        if (ball.x < 80 && std::abs(ball.y - playerY) < 60 && !ball.goingRight) {
            ball.speedX = std::abs(ball.speedX) * 1.05f;
            ball.speedY = (ball.y - playerY) * 0.1f;
            ball.goingRight = true;
        }
        if (ball.x > GAME_WIDTH - 80 && std::abs(ball.y - aiY) < 60 && ball.goingRight) {
            ball.speedX = -std::abs(ball.speedX) * 1.02f;
            ball.speedY = (ball.y - aiY) * 0.08f;
            ball.goingRight = false;
        }

        if (ball.x < 0) { aiScore++; resetBall(true); }
        if (ball.x > GAME_WIDTH) { playerScore++; resetBall(false); }

        SDL_SetRenderDrawColor(renderer, 50, 100, 50, 255);
        SDL_Rect bg = {0, 0, GAME_WIDTH, GAME_HEIGHT};
        SDL_RenderFillRect(renderer, &bg);

        SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
        SDL_RenderDrawLine(renderer, GAME_WIDTH/2, 0, GAME_WIDTH/2, GAME_HEIGHT);
        SDL_RenderDrawLine(renderer, 0, 30, GAME_WIDTH, 30);
        SDL_RenderDrawLine(renderer, 0, GAME_HEIGHT - 30, GAME_WIDTH, GAME_HEIGHT - 30);

        SDL_SetRenderDrawColor(renderer, BLUE.r, BLUE.g, BLUE.b, BLUE.a);
        SDL_Rect playerRacket = {30, (int)playerY - 40, 20, 80};
        SDL_RenderFillRect(renderer, &playerRacket);

        SDL_SetRenderDrawColor(renderer, RED.r, RED.g, RED.b, RED.a);
        SDL_Rect aiRacket = {GAME_WIDTH - 50, (int)aiY - 40, 20, 80};
        SDL_RenderFillRect(renderer, &aiRacket);

        SDL_SetRenderDrawColor(renderer, YELLOW.r, YELLOW.g, YELLOW.b, YELLOW.a);
        drawFilledCircle(renderer, (int)ball.x, (int)ball.y, ball.radius);

        SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
        drawNumber(renderer, playerScore, 150, 50, 40);
        drawNumber(renderer, aiScore, GAME_WIDTH - 200, 50, 40);

        drawCameraFeed(renderer, currentFrame, useCamera);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}
