#pragma once

#include "../common/common.h"

typedef struct session_store {
    int session;
    char username[USERNAME_MAX_LENGTH];
} SessionStore;

typedef struct server_game_state {
    int session;
    //StrPair * wordPair;
    int guessesLeft;
    //char guessedLetters[GUESSED_LETTERS_LENGTH];
} ServerGameState;

struct request {
    int socket_id;
    struct request * next;
};

void interruptHandler(int signal);

int loadAccounts();

int loadWords();

LeaderboardEntry * getScoreForPlayer(char username[USERNAME_MAX_LENGTH]);

void finishUp();

void * handleResponse(struct request * a_request, int thread_id);
