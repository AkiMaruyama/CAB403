#pragma once

#include <stdbool.h>
#include <stdint.h>

#define DEFAULT_PORT 12345
#define BACKLOGIN 10
#define USERNAME_MAX_LENGTH 16
#define PASSWORD_MAX_LENGTH 16

//#define min(a, b) ((a) > (b) ? (b) : (a))

/*
 * this structure defines game state of the client side
 */
typedef struct client_game_state {
    int remainingMines;
    bool won;
} ClientGameState;

/*
 * this structure defines leader board entry
 */
typedef struct leaderboard_entry {
    char username[USERNAME_MAX_LENGTH];
    int wins;
    int games;
} LeaderboardEntry;

/*
 * these packets defines each state for client to server (0 to 127)
 */
#define LOGIN_PACKET 0
#define START_PACKET 1
#define MINESWEEPER_PACKET 2
#define CLOSE_CLIENT_PACKET 3
#define LEADERBOARD_PACKET 4

/*
 * these packets defines each state for server to client (128 to 256)
 */
#define LOGIN_RESPONSE_PACKET ((1 << 7 ) | 0)
#define STATE_RESPONSE_PACKET ((1 << 7 ) | 1)
#define CLOSE_SERVER_PACKET ((1 << 7) | 2)
#define INVALID_MINESWEEPER_PACKET ((1 << 7) | 3)
#define START_LEADERBOARD_PACKET ((1 << 7) | 4)
#define ENTRY_LEADERBOARD_PACKET ((1 << 7) | 5)
#define END_LEADERBOARD_PACKET ((1 << 7) | 6)

/*
 * this structure defines the data of packets
 */
typedef struct data_packet {
    uint8_t type;
    int session;
} DataPacket;

/*
 * this structure defines the login detail
 */
typedef struct login_details_payload {
    char username[USERNAME_MAX_LENGTH];
    char password[PASSWORD_MAX_LENGTH];
} LoginDetailsPayload;

/*
 * this structure defines player's turn detail
 */
typedef struct take_turn_payload {
    char move;
} TakeTurnPayload;

/*
 * this structure defines the login response
 */
typedef struct login_response_payload {
    bool success;
} LoginResponsePayload;

/*
 * this function checks if the port is valid
 */
int validatePort(int port);

/*
 * this function sends a error message
 */
void error(char * msg);
