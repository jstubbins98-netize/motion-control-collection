#ifndef BOXING_GAME_H
#define BOXING_GAME_H

#include "common.h"

void runBoxing(SDL_Renderer* renderer, PoseDetector& poseDetector, cv::VideoCapture& cap,
               bool useCamera, cv::Mat& currentFrame, bool& returnToMenu, Mix_Music* menuMusic);

#endif
