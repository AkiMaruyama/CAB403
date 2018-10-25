#pragma once

#include <stdbool.h>
#include "../common/common.h"

/*
 * this enumeration defines types of screens.
 */
typedef enum screen_type {
    LOGIN,
    MENU,
    LOSE,
    WIN,
    LEADERBOARD,
    GAME
} ScreenType;

/*
 * this function draws "welcome text"
 */
void drawWelcomeMessage();

/*
 * this function checks if the users are valid.
 */
bool isAuthenticateUser(char username[], char password[]);

/*
 * this function draws screens depending on the screen type
 */
void drawScreen(ScreenType screenType);

/*
 * this function controls the minesweeper gaming system.
 */
void gameStart();
