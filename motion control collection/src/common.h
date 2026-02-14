#ifndef COMMON_H
#define COMMON_H

#include <SDL2/SDL.h>
#include <SDL_mixer.h>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

const int GAME_WIDTH = 800;
const int GAME_HEIGHT = 600;
const int CAM_WIDTH = 320;
const int CAM_HEIGHT = 240;
const int WINDOW_WIDTH = GAME_WIDTH + CAM_WIDTH;

struct Color {
    Uint8 r, g, b, a;
};

extern const Color WHITE;
extern const Color BLACK;
extern const Color YELLOW;
extern const Color BLUE;
extern const Color RED;
extern const Color ORANGE;
extern const Color GREEN;
extern const Color PURPLE;
extern const Color PINK;
extern const Color COURT_GREEN;
extern const Color DARK_GREEN;
extern const Color KITCHEN_GREEN;
extern const Color NET_COLOR;

enum GameState {
    MENU,
    PICKLEBALL,
    BOXING,
    TENNIS,
    ARCHERY,
    RHYTHM_DANCE
};

class PoseDetector {
public:
    cv::dnn::Net net;
    bool initialized = false;
    float armCenterY = 0.5f;
    float leftHandX = 0.3f, leftHandY = 0.5f;
    float rightHandX = 0.7f, rightHandY = 0.5f;
    cv::Mat prevFrame;

    bool init();
    float detectArmPosition(cv::Mat& frame);
    void detectBothHands(cv::Mat& frame);
    void motionFallbackBothHands(cv::Mat& frame);
    float motionFallback(cv::Mat& frame);
};

void drawFilledCircle(SDL_Renderer* renderer, int cx, int cy, int radius);
void drawDigit(SDL_Renderer* renderer, int digit, int x, int y, int size);
void drawNumber(SDL_Renderer* renderer, int number, int x, int y, int size);
void drawChar(SDL_Renderer* renderer, char c, int x, int y, int size);
void drawText(SDL_Renderer* renderer, const char* text, int x, int y, int size);
void drawCameraFeed(SDL_Renderer* renderer, cv::Mat& frame, bool useCamera);
void drawInfoPopup(SDL_Renderer* renderer, int gameIndex);

#endif
