#pragma once

#include "../common/common.h"

#include <stdbool.h>

/*
 * this enumeration defines types of screens.
 */
typedef enum screen_type {
    LOGIN,
    MENU,
    GAMEOVER,
    WIN,
    LEADERBOARD,
    GAME
} ScreenType;

/*
 * this function draws "welcome text"
 */
void drawWelcomeText();

/*
 * this function draws screens depending on the screen type
 */
void drawScreen(ScreenType screenType);

/*
 * this function checks if the users are valid.
 */
bool authenticateUser(char username[], char password[]);

/*
 * this function controls the minesweeper gaming system.
 */
void gameStart();
