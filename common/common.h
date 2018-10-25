#pragma once

#include <stdbool.h>
#include <stdint.h>

#define DEFAULT_PORT 12345
#define KEEPLOGIN 10
#define MAX_USERNAME 16
#define MAX_PASSWORD 16

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
    char username[MAX_USERNAME];
    int wins;
    int games;
} LeaderboardEntry;

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
typedef struct login_details {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
} LoginDetails;

/*
 * this structure defines player's turn detail
 */
typedef struct take_turn {
    char move;
} TakeTurn;

/*
 * this structure defines the login response
 */
typedef struct login_response {
    bool success;
} LoginResponse;

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
#define LOGIN_RESPONSE_PACKET ((1 << 7) | 0)
#define STATE_RESPONSE_PACKET ((1 << 7) | 1)
#define CLOSE_SERVER_PACKET ((1 << 7) | 2)
#define INVALID_MINESWEEPER_PACKET ((1 << 7) | 3)
#define START_LEADERBOARD_PACKET ((1 << 7) | 4)
#define ENTRY_LEADERBOARD_PACKET ((1 << 7) | 5)
#define END_LEADERBOARD_PACKET ((1 << 7) | 6)

/*
 * this function sends a error message
 */
void error(char * msg);

/*
 * this function checks if the port is valid
 */
int validatePort(int port);

