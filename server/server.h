#pragma once

#include "../common/common.h"

/*
 * this structure defines the current session
 * including the serssion and the user.
 */
typedef struct session_handle {
    int session;
    char username[USERNAME_MAX_LENGTH];
} SessionHandle;

/*
 * this structure defines the game state on the server side.
 */
typedef struct server_game_state {
    int session;
} ServerGameState;

/*
 * this structure handles the socket pool
 * 
 * THIS IS IMPORTANT AFTER STEP 2
 * 
 */
struct request {
    int socket_id;
    struct request * next;
};

/*
 * this function controlls the interrupt signal to stop the program
 */
void interruptHandler(int signal);

/*
 * this function loads the users account from a text file
 */
int loadAccounts();

/*
 * this function controls the leader board information
 * it's mainly the scores for players
 */
LeaderboardEntry * getScoreForPlayer(char username[USERNAME_MAX_LENGTH]);

/*
 * this function releases all memory captured
 */
void finish();

/*
 * this function controls the thread pool
 */
void * handleResponse(struct request * a_request, int thread_id);
