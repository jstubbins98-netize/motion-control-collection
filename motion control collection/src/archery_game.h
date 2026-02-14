#ifndef ARCHERY_GAME_H
#define ARCHERY_GAME_H

#include "common.h"

void runArchery(SDL_Renderer* renderer, PoseDetector& poseDetector, cv::VideoCapture& cap,
                bool useCamera, cv::Mat& currentFrame, bool& returnToMenu, Mix_Music* menuMusic);

#endif
