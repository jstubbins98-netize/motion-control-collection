#ifndef PICKLEBALL_GAME_H
#define PICKLEBALL_GAME_H

#include "common.h"

void runPickleball(SDL_Renderer* renderer, PoseDetector& poseDetector, cv::VideoCapture& cap, 
                   bool useCamera, cv::Mat& currentFrame, bool& returnToMenu);

#endif
