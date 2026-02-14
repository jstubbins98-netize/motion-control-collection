#ifndef RHYTHM_DANCE_GAME_H
#define RHYTHM_DANCE_GAME_H

#include "common.h"

void runRhythmDance(SDL_Renderer* renderer, PoseDetector& poseDetector, cv::VideoCapture& cap,
                    bool useCamera, cv::Mat& currentFrame, bool& returnToMenu, Mix_Music* bgMusic);

#endif
