#include "common.h"

const Color WHITE = {255, 255, 255, 255};
const Color BLACK = {0, 0, 0, 255};
const Color YELLOW = {255, 255, 0, 255};
const Color BLUE = {30, 144, 255, 255};
const Color RED = {255, 69, 0, 255};
const Color ORANGE = {255, 165, 0, 255};
const Color GREEN = {0, 200, 0, 255};
const Color PURPLE = {128, 0, 128, 255};
const Color PINK = {255, 105, 180, 255};
const Color COURT_GREEN = {76, 153, 0, 255};
const Color DARK_GREEN = {25, 100, 25, 255};
const Color KITCHEN_GREEN = {100, 180, 80, 255};
const Color NET_COLOR = {50, 50, 50, 255};

bool PoseDetector::init() {
    try {
        std::string protoFile = "models/openpose_pose_coco.prototxt";
        std::string weightsFile = "models/pose_iter_440000.caffemodel";
        
        std::ifstream protoTest(protoFile);
        std::ifstream weightsTest(weightsFile);
        if (protoTest.good() && weightsTest.good()) {
            net = cv::dnn::readNetFromCaffe(protoFile, weightsFile);
            net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
            net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
            initialized = true;
            std::cout << "OpenPose model loaded successfully!" << std::endl;
            return true;
        } else {
            std::cout << "OpenPose model files not found. Using motion detection fallback." << std::endl;
            return false;
        }
    } catch (const cv::Exception& e) {
        std::cout << "Failed to load OpenPose: " << e.what() << std::endl;
        return false;
    }
}

float PoseDetector::detectArmPosition(cv::Mat& frame) {
    if (!initialized) {
        return motionFallback(frame);
    }

    cv::Mat inputBlob = cv::dnn::blobFromImage(frame, 1.0 / 255, cv::Size(368, 368), 
                                                cv::Scalar(0, 0, 0), false, false);
    net.setInput(inputBlob);
    cv::Mat output = net.forward();

    int H = output.size[2];
    int W = output.size[3];

    int leftWristIdx = 7;
    int rightWristIdx = 4;

    cv::Mat leftWristHeatmap(H, W, CV_32F, output.ptr(0, leftWristIdx));
    cv::Mat rightWristHeatmap(H, W, CV_32F, output.ptr(0, rightWristIdx));

    cv::Point leftMax, rightMax;
    double leftConf, rightConf;
    cv::minMaxLoc(leftWristHeatmap, nullptr, &leftConf, nullptr, &leftMax);
    cv::minMaxLoc(rightWristHeatmap, nullptr, &rightConf, nullptr, &rightMax);

    float avgY = 0.5f;
    int count = 0;
    
    if (leftConf > 0.1) {
        leftHandX = (float)leftMax.x / W;
        leftHandY = (float)leftMax.y / H;
        avgY += leftHandY;
        count++;
    }
    if (rightConf > 0.1) {
        rightHandX = (float)rightMax.x / W;
        rightHandY = (float)rightMax.y / H;
        avgY += rightHandY;
        count++;
    }
    
    if (count > 0) {
        avgY /= (count + 1);
    }

    armCenterY = armCenterY * 0.7f + avgY * 0.3f;
    return armCenterY;
}

void PoseDetector::detectBothHands(cv::Mat& frame) {
    if (!initialized) {
        motionFallbackBothHands(frame);
        return;
    }
    detectArmPosition(frame);
}

void PoseDetector::motionFallbackBothHands(cv::Mat& frame) {
    if (prevFrame.empty()) {
        prevFrame = frame.clone();
        return;
    }

    cv::Mat gray1, gray2, diff;
    cv::cvtColor(prevFrame, gray1, cv::COLOR_BGR2GRAY);
    cv::cvtColor(frame, gray2, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray1, gray1, cv::Size(11, 11), 0);
    cv::GaussianBlur(gray2, gray2, cv::Size(11, 11), 0);
    cv::absdiff(gray1, gray2, diff);
    cv::threshold(diff, diff, 40, 255, cv::THRESH_BINARY);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::erode(diff, diff, kernel);
    cv::dilate(diff, diff, kernel, cv::Point(-1,-1), 2);

    cv::Mat leftHalf = diff(cv::Rect(0, 0, diff.cols/2, diff.rows));
    cv::Mat rightHalf = diff(cv::Rect(diff.cols/2, 0, diff.cols/2, diff.rows));

    cv::Moments leftMom = cv::moments(leftHalf, true);
    cv::Moments rightMom = cv::moments(rightHalf, true);

    float minMotionArea = 800.0;
    if (leftMom.m00 > minMotionArea) {
        float newX = (leftMom.m10 / leftMom.m00) / diff.cols;
        float newY = (leftMom.m01 / leftMom.m00) / diff.rows;
        leftHandX = leftHandX * 0.6f + newX * 0.4f;
        leftHandY = leftHandY * 0.6f + newY * 0.4f;
    }
    if (rightMom.m00 > minMotionArea) {
        float newX = 0.5f + (rightMom.m10 / rightMom.m00) / diff.cols;
        float newY = (rightMom.m01 / rightMom.m00) / diff.rows;
        rightHandX = rightHandX * 0.6f + newX * 0.4f;
        rightHandY = rightHandY * 0.6f + newY * 0.4f;
    }

    prevFrame = frame.clone();
}

float PoseDetector::motionFallback(cv::Mat& frame) {
    if (prevFrame.empty()) {
        prevFrame = frame.clone();
        return 0.5f;
    }

    cv::Mat gray1, gray2, diff;
    cv::cvtColor(prevFrame, gray1, cv::COLOR_BGR2GRAY);
    cv::cvtColor(frame, gray2, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray1, gray1, cv::Size(11, 11), 0);
    cv::GaussianBlur(gray2, gray2, cv::Size(11, 11), 0);
    cv::absdiff(gray1, gray2, diff);
    cv::threshold(diff, diff, 40, 255, cv::THRESH_BINARY);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::erode(diff, diff, kernel);
    cv::dilate(diff, diff, kernel, cv::Point(-1,-1), 2);

    cv::Moments m = cv::moments(diff, true);
    if (m.m00 > 800) {
        float centerY = (float)(m.m01 / m.m00) / diff.rows;
        armCenterY = armCenterY * 0.8f + centerY * 0.2f;
    }

    prevFrame = frame.clone();
    return armCenterY;
}

void drawFilledCircle(SDL_Renderer* renderer, int cx, int cy, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx*dx + dy*dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
            }
        }
    }
}

void drawDigit(SDL_Renderer* renderer, int digit, int x, int y, int size) {
    bool segments[10][7] = {
        {1,1,1,1,1,1,0}, {0,1,1,0,0,0,0}, {1,1,0,1,1,0,1}, {1,1,1,1,0,0,1},
        {0,1,1,0,0,1,1}, {1,0,1,1,0,1,1}, {1,0,1,1,1,1,1}, {1,1,1,0,0,0,0},
        {1,1,1,1,1,1,1}, {1,1,1,1,0,1,1}
    };
    
    int t = size / 8;
    if (segments[digit][0]) { SDL_Rect r = {x, y, size, t}; SDL_RenderFillRect(renderer, &r); }
    if (segments[digit][1]) { SDL_Rect r = {x + size - t, y, t, size/2}; SDL_RenderFillRect(renderer, &r); }
    if (segments[digit][2]) { SDL_Rect r = {x + size - t, y + size/2, t, size/2}; SDL_RenderFillRect(renderer, &r); }
    if (segments[digit][3]) { SDL_Rect r = {x, y + size - t, size, t}; SDL_RenderFillRect(renderer, &r); }
    if (segments[digit][4]) { SDL_Rect r = {x, y + size/2, t, size/2}; SDL_RenderFillRect(renderer, &r); }
    if (segments[digit][5]) { SDL_Rect r = {x, y, t, size/2}; SDL_RenderFillRect(renderer, &r); }
    if (segments[digit][6]) { SDL_Rect r = {x, y + size/2 - t/2, size, t}; SDL_RenderFillRect(renderer, &r); }
}

void drawNumber(SDL_Renderer* renderer, int number, int x, int y, int size) {
    if (number < 10) {
        drawDigit(renderer, number, x, y, size);
    } else {
        drawDigit(renderer, number / 10, x, y, size);
        drawDigit(renderer, number % 10, x + size + 5, y, size);
    }
}

void drawChar(SDL_Renderer* renderer, char c, int x, int y, int size) {
    int t = size / 5;
    if (t < 1) t = 1;
    
    c = toupper(c);
    
    switch (c) {
        case 'A':
            SDL_RenderDrawLine(renderer, x + size/2, y, x, y + size);
            SDL_RenderDrawLine(renderer, x + size/2, y, x + size, y + size);
            SDL_RenderDrawLine(renderer, x + size/4, y + size/2, x + 3*size/4, y + size/2);
            for (int i = 0; i < t; i++) {
                SDL_RenderDrawLine(renderer, x + size/2, y + i, x + i, y + size);
                SDL_RenderDrawLine(renderer, x + size/2, y + i, x + size - i, y + size);
            }
            break;
        case 'B':
            { SDL_Rect r = {x, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y, size - t, t}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y + size/2 - t/2, size - t, t}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y + size - t, size - t, t}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x + size - t, y, t, size/2}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x + size - t, y + size/2, t, size/2}; SDL_RenderFillRect(renderer, &r); }
            break;
        case 'C':
            { SDL_Rect r = {x, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y, size, t}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y + size - t, size, t}; SDL_RenderFillRect(renderer, &r); }
            break;
        case 'D':
            { SDL_Rect r = {x, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y, size - t, t}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y + size - t, size - t, t}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x + size - t, y + t, t, size - 2*t}; SDL_RenderFillRect(renderer, &r); }
            break;
        case 'E':
            { SDL_Rect r = {x, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y, size, t}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y + size/2 - t/2, size*2/3, t}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y + size - t, size, t}; SDL_RenderFillRect(renderer, &r); }
            break;
        case 'F':
            { SDL_Rect r = {x, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y, size, t}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y + size/2 - t/2, size*2/3, t}; SDL_RenderFillRect(renderer, &r); }
            break;
        case 'G':
            { SDL_Rect r = {x, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y, size, t}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y + size - t, size, t}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x + size - t, y + size/2, t, size/2}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x + size/2, y + size/2, size/2, t}; SDL_RenderFillRect(renderer, &r); }
            break;
        case 'H':
            { SDL_Rect r = {x, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x + size - t, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y + size/2 - t/2, size, t}; SDL_RenderFillRect(renderer, &r); }
            break;
        case 'I':
            { SDL_Rect r = {x, y, size, t}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x + size/2 - t/2, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y + size - t, size, t}; SDL_RenderFillRect(renderer, &r); }
            break;
        case 'J':
            { SDL_Rect r = {x + size - t, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y + size - t, size, t}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y + size/2, t, size/2}; SDL_RenderFillRect(renderer, &r); }
            break;
        case 'K':
            { SDL_Rect r = {x, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            SDL_RenderDrawLine(renderer, x + t, y + size/2, x + size, y);
            SDL_RenderDrawLine(renderer, x + t, y + size/2, x + size, y + size);
            for (int i = 0; i < t; i++) {
                SDL_RenderDrawLine(renderer, x + t, y + size/2 + i, x + size, y + i);
                SDL_RenderDrawLine(renderer, x + t, y + size/2 - i, x + size, y + size - i);
            }
            break;
        case 'L':
            { SDL_Rect r = {x, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y + size - t, size, t}; SDL_RenderFillRect(renderer, &r); }
            break;
        case 'M':
            { SDL_Rect r = {x, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x + size - t, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            for (int i = 0; i < t; i++) {
                SDL_RenderDrawLine(renderer, x + t, y, x + size/2, y + size/3 + i);
                SDL_RenderDrawLine(renderer, x + size - t, y, x + size/2, y + size/3 + i);
            }
            break;
        case 'N':
            { SDL_Rect r = {x, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x + size - t, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            for (int i = 0; i < t; i++) {
                SDL_RenderDrawLine(renderer, x + t, y + i, x + size - t, y + size - i);
            }
            break;
        case 'O':
            { SDL_Rect r = {x, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x + size - t, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y, size, t}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y + size - t, size, t}; SDL_RenderFillRect(renderer, &r); }
            break;
        case 'P':
            { SDL_Rect r = {x, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y, size, t}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x + size - t, y, t, size/2}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y + size/2 - t/2, size, t}; SDL_RenderFillRect(renderer, &r); }
            break;
        case 'Q':
            { SDL_Rect r = {x, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x + size - t, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y, size, t}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y + size - t, size, t}; SDL_RenderFillRect(renderer, &r); }
            SDL_RenderDrawLine(renderer, x + size/2, y + size/2, x + size, y + size);
            break;
        case 'R':
            { SDL_Rect r = {x, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y, size, t}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x + size - t, y, t, size/2}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y + size/2 - t/2, size, t}; SDL_RenderFillRect(renderer, &r); }
            for (int i = 0; i < t; i++) {
                SDL_RenderDrawLine(renderer, x + size/2, y + size/2, x + size, y + size - i);
            }
            break;
        case 'S':
            { SDL_Rect r = {x, y, size, t}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y, t, size/2}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y + size/2 - t/2, size, t}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x + size - t, y + size/2, t, size/2}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y + size - t, size, t}; SDL_RenderFillRect(renderer, &r); }
            break;
        case 'T':
            { SDL_Rect r = {x, y, size, t}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x + size/2 - t/2, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            break;
        case 'U':
            { SDL_Rect r = {x, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x + size - t, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y + size - t, size, t}; SDL_RenderFillRect(renderer, &r); }
            break;
        case 'V':
            for (int i = 0; i < t; i++) {
                SDL_RenderDrawLine(renderer, x + i, y, x + size/2, y + size);
                SDL_RenderDrawLine(renderer, x + size - i, y, x + size/2, y + size);
            }
            break;
        case 'W':
            { SDL_Rect r = {x, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x + size - t, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y + size - t, size, t}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x + size/2 - t/2, y + size/2, t, size/2}; SDL_RenderFillRect(renderer, &r); }
            break;
        case 'X':
            for (int i = 0; i < t; i++) {
                SDL_RenderDrawLine(renderer, x + i, y, x + size, y + size - i);
                SDL_RenderDrawLine(renderer, x + size - i, y, x, y + size - i);
            }
            break;
        case 'Y':
            for (int i = 0; i < t; i++) {
                SDL_RenderDrawLine(renderer, x + i, y, x + size/2, y + size/2);
                SDL_RenderDrawLine(renderer, x + size - i, y, x + size/2, y + size/2);
            }
            { SDL_Rect r = {x + size/2 - t/2, y + size/2, t, size/2}; SDL_RenderFillRect(renderer, &r); }
            break;
        case 'Z':
            { SDL_Rect r = {x, y, size, t}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x, y + size - t, size, t}; SDL_RenderFillRect(renderer, &r); }
            for (int i = 0; i < t; i++) {
                SDL_RenderDrawLine(renderer, x + size, y + i, x, y + size - i);
            }
            break;
        case ' ':
            break;
        case '-':
            { SDL_Rect r = {x + size/4, y + size/2 - t/2, size/2, t}; SDL_RenderFillRect(renderer, &r); }
            break;
        case '/':
            for (int i = 0; i < t; i++) {
                SDL_RenderDrawLine(renderer, x + size - i, y, x + i, y + size);
            }
            break;
        case ':':
            drawFilledCircle(renderer, x + size/2, y + size/3, t);
            drawFilledCircle(renderer, x + size/2, y + 2*size/3, t);
            break;
        case '.':
            drawFilledCircle(renderer, x + size/2, y + size - t, t);
            break;
        case '(':
            { SDL_Rect r = {x + size/2, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x + size/4, y, size/4, t}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x + size/4, y + size - t, size/4, t}; SDL_RenderFillRect(renderer, &r); }
            break;
        case ')':
            { SDL_Rect r = {x + size/2 - t, y, t, size}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x + size/2, y, size/4, t}; SDL_RenderFillRect(renderer, &r); }
            { SDL_Rect r = {x + size/2, y + size - t, size/4, t}; SDL_RenderFillRect(renderer, &r); }
            break;
        default:
            if (c >= '0' && c <= '9') {
                drawDigit(renderer, c - '0', x, y, size);
            }
            break;
    }
}

void drawText(SDL_Renderer* renderer, const char* text, int x, int y, int size) {
    int spacing = size + size/4;
    int cx = x;
    for (int i = 0; text[i] != '\0'; i++) {
        drawChar(renderer, text[i], cx, y, size);
        cx += spacing;
    }
}

void drawInfoPopup(SDL_Renderer* renderer, int gameIndex) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
    SDL_Rect overlay = {0, 0, GAME_WIDTH, GAME_HEIGHT};
    SDL_RenderFillRect(renderer, &overlay);
    
    SDL_SetRenderDrawColor(renderer, 40, 40, 60, 255);
    SDL_Rect popup = {50, 50, GAME_WIDTH - 100, GAME_HEIGHT - 100};
    SDL_RenderFillRect(renderer, &popup);
    
    SDL_SetRenderDrawColor(renderer, YELLOW.r, YELLOW.g, YELLOW.b, YELLOW.a);
    for (int i = 0; i < 3; i++) {
        SDL_Rect border = {50 + i, 50 + i, GAME_WIDTH - 100 - 2*i, GAME_HEIGHT - 100 - 2*i};
        SDL_RenderDrawRect(renderer, &border);
    }
    
    const char* titles[] = {"PICKLEBALL", "BOXING", "TENNIS", "ARCHERY", "RHYTHM DANCE"};
    Color colors[] = {GREEN, RED, YELLOW, ORANGE, PINK};
    
    SDL_SetRenderDrawColor(renderer, colors[gameIndex].r, colors[gameIndex].g, colors[gameIndex].b, 255);
    drawText(renderer, titles[gameIndex], 80, 70, 20);
    
    SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
    
    int lineY = 110;
    int lineH = 22;
    
    switch (gameIndex) {
        case 0:
            drawText(renderer, "CONTROLS:", 80, lineY, 14); lineY += lineH + 5;
            drawText(renderer, "MOVE ARMS UP/DOWN - CONTROL PADDLE", 80, lineY, 12); lineY += lineH;
            drawText(renderer, "W/S OR ARROWS - KEYBOARD FALLBACK", 80, lineY, 12); lineY += lineH;
            drawText(renderer, "SPACE - SERVE BALL", 80, lineY, 12); lineY += lineH;
            drawText(renderer, "R - RESET SCORE", 80, lineY, 12); lineY += lineH + 10;
            drawText(renderer, "HOW TO PLAY:", 80, lineY, 14); lineY += lineH + 5;
            drawText(renderer, "HIT THE BALL PAST THE AI OPPONENT", 80, lineY, 12); lineY += lineH;
            drawText(renderer, "TO SCORE POINTS. FIRST TO WIN.", 80, lineY, 12);
            break;
        case 1:
            drawText(renderer, "CONTROLS:", 80, lineY, 14); lineY += lineH + 5;
            drawText(renderer, "LEFT HAND - PUNCH LEFT TARGETS", 80, lineY, 12); lineY += lineH;
            drawText(renderer, "RIGHT HAND - PUNCH RIGHT TARGETS", 80, lineY, 12); lineY += lineH;
            drawText(renderer, "WASD - MOVE LEFT HAND (FALLBACK)", 80, lineY, 12); lineY += lineH;
            drawText(renderer, "MOUSE - MOVE RIGHT HAND (FALLBACK)", 80, lineY, 12); lineY += lineH + 10;
            drawText(renderer, "HOW TO PLAY:", 80, lineY, 14); lineY += lineH + 5;
            drawText(renderer, "PUNCH TARGETS AS THEY APPEAR.", 80, lineY, 12); lineY += lineH;
            drawText(renderer, "SCORE POINTS FOR EACH HIT.", 80, lineY, 12);
            break;
        case 2:
            drawText(renderer, "CONTROLS:", 80, lineY, 14); lineY += lineH + 5;
            drawText(renderer, "MOVE ARMS UP/DOWN - CONTROL RACKET", 80, lineY, 12); lineY += lineH;
            drawText(renderer, "W/S OR ARROWS - KEYBOARD FALLBACK", 80, lineY, 12); lineY += lineH + 10;
            drawText(renderer, "HOW TO PLAY:", 80, lineY, 14); lineY += lineH + 5;
            drawText(renderer, "POSITION YOUR RACKET TO HIT THE", 80, lineY, 12); lineY += lineH;
            drawText(renderer, "BALL BACK OVER THE NET. SCORE", 80, lineY, 12); lineY += lineH;
            drawText(renderer, "WHEN YOUR OPPONENT MISSES.", 80, lineY, 12);
            break;
        case 3:
            drawText(renderer, "CONTROLS:", 80, lineY, 14); lineY += lineH + 5;
            drawText(renderer, "MOVE ARMS - AIM UP/DOWN", 80, lineY, 12); lineY += lineH;
            drawText(renderer, "DRAW MOTION - PULL HANDS APART", 80, lineY, 12); lineY += lineH;
            drawText(renderer, "RELEASE - BRING HANDS TOGETHER", 80, lineY, 12); lineY += lineH;
            drawText(renderer, "W/S - AIM (FALLBACK)", 80, lineY, 12); lineY += lineH;
            drawText(renderer, "SPACE HOLD/RELEASE - SHOOT", 80, lineY, 12); lineY += lineH + 10;
            drawText(renderer, "HOW TO PLAY:", 80, lineY, 14); lineY += lineH + 5;
            drawText(renderer, "AIM AT TARGETS AND SHOOT ARROWS.", 80, lineY, 12); lineY += lineH;
            drawText(renderer, "HIT THE CENTER FOR MAX POINTS.", 80, lineY, 12);
            break;
        case 4:
            drawText(renderer, "CONTROLS:", 80, lineY, 14); lineY += lineH + 5;
            drawText(renderer, "FULL BODY - MATCH TARGET POSES", 80, lineY, 12); lineY += lineH;
            drawText(renderer, "BOTH ARMS TRACKED", 80, lineY, 12); lineY += lineH + 10;
            drawText(renderer, "HOW TO PLAY:", 80, lineY, 14); lineY += lineH + 5;
            drawText(renderer, "WATCH THE TARGET POSES ON SCREEN.", 80, lineY, 12); lineY += lineH;
            drawText(renderer, "MATCH THEM IN TIME WITH THE MUSIC.", 80, lineY, 12); lineY += lineH;
            drawText(renderer, "CHAIN POSES FOR COMBO BONUS.", 80, lineY, 12);
            break;
    }
    
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    drawText(renderer, "PRESS ANY KEY TO CLOSE", 200, GAME_HEIGHT - 90, 12);
}

void drawCameraFeed(SDL_Renderer* renderer, cv::Mat& frame, bool useCamera) {
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_Rect camArea = {GAME_WIDTH, 0, CAM_WIDTH, CAM_HEIGHT};
    SDL_RenderFillRect(renderer, &camArea);

    if (!frame.empty()) {
        cv::Mat resized;
        cv::resize(frame, resized, cv::Size(CAM_WIDTH, CAM_HEIGHT));
        cv::cvtColor(resized, resized, cv::COLOR_BGR2RGB);

        SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
            resized.data, CAM_WIDTH, CAM_HEIGHT, 24, resized.step,
            0x0000FF, 0x00FF00, 0xFF0000, 0
        );
        
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (texture) {
                SDL_Rect destRect = {GAME_WIDTH, 0, CAM_WIDTH, CAM_HEIGHT};
                SDL_RenderCopy(renderer, texture, nullptr, &destRect);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    } else {
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        int cx = GAME_WIDTH + CAM_WIDTH / 2;
        int cy = CAM_HEIGHT / 2;
        drawFilledCircle(renderer, cx, cy - 10, 15);
        SDL_Rect body = {cx - 25, cy + 10, 50, 30};
        SDL_RenderFillRect(renderer, &body);
    }

    SDL_SetRenderDrawColor(renderer, BLUE.r, BLUE.g, BLUE.b, BLUE.a);
    for (int i = 0; i < 3; i++) {
        SDL_Rect border = {GAME_WIDTH + i, i, CAM_WIDTH - 2*i, CAM_HEIGHT - 2*i};
        SDL_RenderDrawRect(renderer, &border);
    }

    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_Rect infoArea = {GAME_WIDTH, CAM_HEIGHT, CAM_WIDTH, GAME_HEIGHT - CAM_HEIGHT};
    SDL_RenderFillRect(renderer, &infoArea);
    
    if (useCamera) {
        SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
    }
    drawFilledCircle(renderer, GAME_WIDTH + CAM_WIDTH - 30, CAM_HEIGHT + 30, 8);
}
