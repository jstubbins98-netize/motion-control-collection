#include "menu.h"

void drawMenu(SDL_Renderer* renderer, int selectedGame) {
    SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255);
    SDL_Rect bg = {0, 0, GAME_WIDTH, GAME_HEIGHT};
    SDL_RenderFillRect(renderer, &bg);

    SDL_SetRenderDrawColor(renderer, YELLOW.r, YELLOW.g, YELLOW.b, YELLOW.a);
    drawText(renderer, "MOTION CONTROL", GAME_WIDTH/2 - 140, 30, 18);
    drawText(renderer, "COLLECTION", GAME_WIDTH/2 - 100, 55, 18);

    const char* gameNames[] = {"PICKLEBALL", "BOXING", "TENNIS", "ARCHERY", "RHYTHM DANCE"};
    Color colors[] = {GREEN, RED, YELLOW, ORANGE, PINK};
    
    for (int i = 0; i < 5; i++) {
        int yPos = 130 + i * 75;
        
        if (i == selectedGame) {
            SDL_SetRenderDrawColor(renderer, colors[i].r, colors[i].g, colors[i].b, 255);
            SDL_Rect highlight = {50, yPos - 5, GAME_WIDTH - 100, 55};
            SDL_RenderFillRect(renderer, &highlight);
            SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a);
        } else {
            SDL_SetRenderDrawColor(renderer, 50, 50, 70, 255);
            SDL_Rect gameBox = {60, yPos, GAME_WIDTH - 120, 45};
            SDL_RenderFillRect(renderer, &gameBox);
            
            SDL_SetRenderDrawColor(renderer, colors[i].r, colors[i].g, colors[i].b, 255);
            for (int b = 0; b < 2; b++) {
                SDL_Rect border = {60 + b, yPos + b, GAME_WIDTH - 120 - 2*b, 45 - 2*b};
                SDL_RenderDrawRect(renderer, &border);
            }
        }
        
        int numX = 90;
        drawDigit(renderer, i + 1, numX, yPos + 12, 20);
        
        SDL_SetRenderDrawColor(renderer, i == selectedGame ? BLACK.r : colors[i].r, 
                               i == selectedGame ? BLACK.g : colors[i].g, 
                               i == selectedGame ? BLACK.b : colors[i].b, 255);
        
        drawText(renderer, gameNames[i], numX + 45, yPos + 12, 16);
    }

    SDL_SetRenderDrawColor(renderer, 120, 120, 140, 255);
    drawText(renderer, "W/S: SELECT", 100, GAME_HEIGHT - 55, 10);
    drawText(renderer, "ENTER: PLAY", 300, GAME_HEIGHT - 55, 10);
    drawText(renderer, "I: INFO", 500, GAME_HEIGHT - 55, 10);
    drawText(renderer, "Q: QUIT", 620, GAME_HEIGHT - 55, 10);
}
