#ifndef TENNIS_GAME_H
#define TENNIS_GAME_H

#include "common.h"

void runTennis(SDL_Renderer* renderer, PoseDetector& poseDetector, cv::VideoCapture& cap,
               bool useCamera, cv::Mat& currentFrame, bool& returnToMenu);

#endif
