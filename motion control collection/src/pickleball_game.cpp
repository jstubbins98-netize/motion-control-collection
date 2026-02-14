#include "pickleball_game.h"

class Paddle {
public:
    float x, y;
    int width = 15;
    int height = 80;
    float speed = 8.0f;
    bool isPlayer;
    Color color;

    Paddle(float x, float y, bool isPlayer) : x(x), y(y), isPlayer(isPlayer) {
        color = isPlayer ? BLUE : RED;
    }

    void move(float targetY) {
        if (y < targetY) {
            y = std::min(y + speed, targetY);
        } else if (y > targetY) {
            y = std::max(y - speed, targetY);
        }
        y = std::max(60.0f, std::min((float)(GAME_HEIGHT - 60 - height), y));
    }

    void draw(SDL_Renderer* renderer) {
        SDL_Rect rect = {(int)x, (int)y, width, height};
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
        SDL_RenderDrawRect(renderer, &rect);
    }
};

class Ball {
public:
    float x, y;
    int radius = 12;
    float speedX, speedY;
    float maxSpeed = 15.0f;
    std::vector<std::pair<float, float>> trail;

    Ball() { reset(); }

    void reset() {
        x = GAME_WIDTH / 2.0f;
        y = GAME_HEIGHT / 2.0f;
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> angleDist(-0.5, 0.5);
        std::uniform_int_distribution<> dirDist(0, 1);
        
        float angle = angleDist(gen);
        int direction = dirDist(gen) == 0 ? -1 : 1;
        speedX = 7.0f * direction;
        speedY = 5.0f * angle;
        trail.clear();
    }

    void update() {
        trail.push_back({x, y});
        if (trail.size() > 10) trail.erase(trail.begin());
        x += speedX;
        y += speedY;
        if (y - radius < 60 || y + radius > GAME_HEIGHT - 60) {
            speedY *= -1;
            y = std::max(60.0f + radius, std::min((float)(GAME_HEIGHT - 60 - radius), y));
        }
    }

    bool checkPaddleCollision(Paddle& paddle) {
        if (paddle.x < x && x < paddle.x + paddle.width + radius &&
            paddle.y - radius < y && y < paddle.y + paddle.height + radius) {
            float relativeY = (y - paddle.y) / paddle.height - 0.5f;
            float bounceAngle = relativeY * 1.2f;
            float speed = std::sqrt(speedX * speedX + speedY * speedY);
            speed = std::min(speed * 1.02f, maxSpeed);
            if (paddle.isPlayer) {
                speedX = std::abs(speed * std::cos(bounceAngle));
            } else {
                speedX = -std::abs(speed * std::cos(bounceAngle));
            }
            speedY = speed * std::sin(bounceAngle);
            if (paddle.isPlayer) {
                x = paddle.x + paddle.width + radius;
            } else {
                x = paddle.x - radius;
            }
            return true;
        }
        return false;
    }

    void draw(SDL_Renderer* renderer) {
        for (size_t i = 0; i < trail.size(); i++) {
            int trailRadius = std::max(2, (int)(radius * i / trail.size()));
            SDL_SetRenderDrawColor(renderer, 255, 255, 100, 255);
            drawFilledCircle(renderer, (int)trail[i].first, (int)trail[i].second, trailRadius);
        }
        SDL_SetRenderDrawColor(renderer, YELLOW.r, YELLOW.g, YELLOW.b, YELLOW.a);
        drawFilledCircle(renderer, (int)x, (int)y, radius);
    }
};

class AIOpponent {
public:
    Paddle& paddle;
    Ball& ball;
    float targetY;
    float difficulty = 0.7f;

    AIOpponent(Paddle& p, Ball& b) : paddle(p), ball(b), targetY(GAME_HEIGHT / 2.0f) {}

    void update() {
        if (ball.speedX > 0) {
            float predictedY = predictBallY();
            targetY = predictedY - paddle.height / 2.0f;
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> noiseDist(-30, 30);
            targetY += noiseDist(gen) * (1 - difficulty);
        } else {
            targetY = GAME_HEIGHT / 2.0f - paddle.height / 2.0f;
        }
        paddle.move(targetY);
    }

    float predictBallY() {
        float px = ball.x, py = ball.y;
        float vx = ball.speedX, vy = ball.speedY;
        while (px < paddle.x) {
            px += vx;
            py += vy;
            if (py < 60 + ball.radius || py > GAME_HEIGHT - 60 - ball.radius) vy *= -1;
        }
        return py;
    }
};

static void drawPickleballCourt(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, DARK_GREEN.r, DARK_GREEN.g, DARK_GREEN.b, DARK_GREEN.a);
    SDL_Rect background = {0, 0, GAME_WIDTH, GAME_HEIGHT};
    SDL_RenderFillRect(renderer, &background);

    SDL_SetRenderDrawColor(renderer, COURT_GREEN.r, COURT_GREEN.g, COURT_GREEN.b, COURT_GREEN.a);
    SDL_Rect court = {50, 60, GAME_WIDTH - 100, GAME_HEIGHT - 120};
    SDL_RenderFillRect(renderer, &court);

    SDL_SetRenderDrawColor(renderer, KITCHEN_GREEN.r, KITCHEN_GREEN.g, KITCHEN_GREEN.b, KITCHEN_GREEN.a);
    SDL_Rect leftKitchen = {50, 60, 150, GAME_HEIGHT - 120};
    SDL_Rect rightKitchen = {GAME_WIDTH - 200, 60, 150, GAME_HEIGHT - 120};
    SDL_RenderFillRect(renderer, &leftKitchen);
    SDL_RenderFillRect(renderer, &rightKitchen);

    SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
    SDL_RenderDrawRect(renderer, &court);
    SDL_RenderDrawLine(renderer, GAME_WIDTH/2, 60, GAME_WIDTH/2, GAME_HEIGHT - 60);
    SDL_RenderDrawLine(renderer, 200, 60, 200, GAME_HEIGHT - 60);
    SDL_RenderDrawLine(renderer, GAME_WIDTH - 200, 60, GAME_WIDTH - 200, GAME_HEIGHT - 60);

    SDL_SetRenderDrawColor(renderer, NET_COLOR.r, NET_COLOR.g, NET_COLOR.b, NET_COLOR.a);
    for (int i = 0; i < 8; i++) {
        SDL_Rect netPart = {GAME_WIDTH/2 - 3, 60 + i * ((GAME_HEIGHT - 120) / 8), 6, (GAME_HEIGHT - 120) / 8 - 5};
        SDL_RenderFillRect(renderer, &netPart);
    }
}

static void drawScore(SDL_Renderer* renderer, int playerScore, int aiScore) {
    SDL_SetRenderDrawColor(renderer, BLUE.r, BLUE.g, BLUE.b, BLUE.a);
    drawNumber(renderer, playerScore, 150, 20, 30);
    SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
    SDL_Rect dash = {GAME_WIDTH/2 - 10, 25, 20, 5};
    SDL_RenderFillRect(renderer, &dash);
    SDL_SetRenderDrawColor(renderer, RED.r, RED.g, RED.b, RED.a);
    drawNumber(renderer, aiScore, GAME_WIDTH - 200, 20, 30);
}

void runPickleball(SDL_Renderer* renderer, PoseDetector& poseDetector, cv::VideoCapture& cap, 
                   bool useCamera, cv::Mat& currentFrame, bool& returnToMenu) {
    Paddle player(70, GAME_HEIGHT / 2.0f - 40, true);
    Paddle opponent(GAME_WIDTH - 85, GAME_HEIGHT / 2.0f - 40, false);
    Ball ball;
    AIOpponent ai(opponent, ball);
    
    int playerScore = 0, aiScore = 0;
    bool gameStarted = false;
    bool running = true;

    while (running && !returnToMenu) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) { running = false; returnToMenu = false; }
            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE: returnToMenu = true; break;
                    case SDLK_q: returnToMenu = true; break;
                    case SDLK_SPACE: if (!gameStarted) { gameStarted = true; ball.reset(); } break;
                    case SDLK_r: ball.reset(); playerScore = 0; aiScore = 0; break;
                }
            }
        }

        const Uint8* keys = SDL_GetKeyboardState(NULL);
        if (!useCamera) {
            if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) player.move(player.y - 10);
            if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) player.move(player.y + 10);
        }

        if (useCamera && cap.read(currentFrame)) {
            cv::flip(currentFrame, currentFrame, 1);
            float armY = poseDetector.detectArmPosition(currentFrame);
            if (gameStarted) {
                float paddleY = armY * (GAME_HEIGHT - 200) + 60;
                player.move(paddleY);
            }
        }

        if (gameStarted) {
            ball.update();
            ai.update();
            ball.checkPaddleCollision(player);
            ball.checkPaddleCollision(opponent);
            
            if (ball.x < 0) { aiScore++; ball.reset(); }
            if (ball.x > GAME_WIDTH) { playerScore++; ball.reset(); }
        }

        drawPickleballCourt(renderer);
        drawScore(renderer, playerScore, aiScore);
        ball.draw(renderer);
        player.draw(renderer);
        opponent.draw(renderer);
        drawCameraFeed(renderer, currentFrame, useCamera);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}
