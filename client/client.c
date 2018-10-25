#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "client.h"

/* NEW ADDED 10/25/18 */
#define RANDOM_NUMBER_SEED 42
#define NUM_TILES_X 9
#define NUM_TILES_Y 9
#define NUM_MINES 10

char server_board[NUM_TILES_X][NUM_TILES_Y];
char client_board[NUM_TILES_X][NUM_TILES_Y];
char *mine = "*";
char *revealed = "x";
char *flag = "+";
char *space = "-";
char option_input;

int mine_positions[NUM_MINES][2];
int remaining_mines = NUM_MINES;
int x_input, y_input, menu_option;

bool valid_option, playing_game;
bool tile_revealed[NUM_TILES_X][NUM_TILES_Y];

typedef struct {
    int adjacent_mines;
    bool revealed;
    bool is_mine;
} Tile;

typedef struct {
    Tile tiles[NUM_TILES_X][NUM_TILES_Y];
    bool running;
} GameState;

bool tile_contains_mine(int x, int y) {
    if (server_board[x][y] == *mine) {
        return true;
    } else {
        return false;
    }
}

void place_mines() {
    for (int i = 0; i < NUM_MINES; i++) {
        int x, y;
        do {
            x = rand() % NUM_TILES_X;
            y = rand() % NUM_TILES_Y;
        } while (tile_contains_mine(x, y));
        mine_positions[i][0] = x;
        mine_positions[i][1] = y;
        server_board[x][y] = *mine;
    }
}

bool is_valid(int x, int y) {
    return (x >= 0) && (x < NUM_TILES_X) && (y >= 0) && (y < NUM_TILES_Y);
}

int check_tile(int x, int y) {
    int surrounding_tiles = 0;
    if (tile_contains_mine(x, y)) {
        surrounding_tiles = -1;
    } else {
        if (tile_contains_mine(x + 1, y) && is_valid(x + 1, y)) {
            surrounding_tiles++;
        } if (tile_contains_mine(x - 1, y) && is_valid(x - 1, y)) {
            surrounding_tiles++;
        } if (tile_contains_mine(x, y - 1) && is_valid(x, y - 1)) {
            surrounding_tiles++;
        } if (tile_contains_mine(x, y + 1) && is_valid(x, y + 1)) {
            surrounding_tiles++;
        } if (tile_contains_mine(x + 1, y + 1) && is_valid(x + 1, y + 1)) {
            surrounding_tiles++;
        } if (tile_contains_mine(x - 1, y + 1) && is_valid(x - 1, y + 1)) {
            surrounding_tiles++;
        } if (tile_contains_mine(x + 1, y - 1) && is_valid(x + 1, y - 1)) {
            surrounding_tiles++;
        } if (tile_contains_mine(x - 1, y - 1) && is_valid(x - 1, y - 1)) {
            surrounding_tiles++;
        }
    }
    return surrounding_tiles;
}

void initialise_client_board() {
    for (int i = 0; i < NUM_TILES_X; i++) {
        for (int j = 0; j < NUM_TILES_Y; j++) {
            client_board[j][i] = *space;
            server_board[j][i] = *space;
        }
    }
}

void drawboard() {
    printf("\n   123456789\n");
    printf("-------------\n"); 
    for (int i = 0; i < NUM_TILES_X; i++) {
        for (int j = 0; j < NUM_TILES_Y; j++) {
            if (j == 0) {
                printf("%d| ", i + 1);
            }
            printf("%c", client_board[j][i]);
        }
        printf("\n");
    }
    printf("\n");
}

void reveal_tile(int x, int y) {
    int tile_no = check_tile(x, y);
    char tile_no_c = '0' + tile_no;
    if (is_valid(x, y)) {
        client_board[x][y] = tile_no_c;
    }
}

void open_safe_tiles(int x, int y) {
    if ((check_tile(x, y)) == 0) {
        reveal_tile(x - 1, y);
        if (server_board[x - 1][y] == '0' && tile_revealed[x - 1][y] == false && is_valid(x - 1, y)) {
            tile_revealed[x - 1][y] = true;
            open_safe_tiles(x - 1, y);
        }

        reveal_tile(x - 1, y - 1);
        if (server_board[x - 1][y - 1] == '0' && tile_revealed[x - 1][y - 1] == false && is_valid(x - 1, y -1)) {
            tile_revealed[x - 1][y - 1] = true;
            open_safe_tiles(x - 1, y - 1);
        }

        reveal_tile(x, y - 1);
        if (server_board[x][y - 1] == '0' && tile_revealed[x][y - 1] == false && is_valid(x, y - 1)) {
            tile_revealed[x][y - 1] = true;
            open_safe_tiles(x, y - 1);
        }

        reveal_tile(x + 1, y - 1);
        if (server_board[x + 1][y - 1] == '0' && tile_revealed[x + 1][y - 1] == false && is_valid(x + 1, y - 1)) {
            tile_revealed[x + 1][y - 1] = true;
            open_safe_tiles(x + 1, y - 1);
        }

        reveal_tile(x + 1, y);
        if (server_board[x + 1][y] == '0' && tile_revealed[x + 1][y] == false && is_valid(x + 1, y)) {
            tile_revealed[x + 1][y] = true;
            open_safe_tiles(x + 1, y);
        }

        reveal_tile(x + 1, y + 1);
        if (server_board[x + 1][y + 1] == '0' && tile_revealed[x + 1][y + 1] == false && is_valid(x + 1, y + 1)) {
            tile_revealed[x + 1][y + 1] = true;
            open_safe_tiles(x + 1, y + 1);
        }

        reveal_tile(x, y + 1);
        if (server_board[x][y + 1] == '0' && tile_revealed[x][y + 1] == false && is_valid(x, y + 1)) {
            tile_revealed[x][y + 1] = true;
            open_safe_tiles(x, y + 1);
        }

        reveal_tile(x - 1, y + 1);
        if (server_board[x - 1][y + 1] == '0' && tile_revealed[x - 1][y + 1] == false && is_valid(x - 1, y + 1)) {
            tile_revealed[x - 1][y + 1] = true;
            open_safe_tiles(x - 1, y + 1);
        }
    }
}

void reveal_mines() {
    for (int i = 0; i < NUM_MINES; i++) {
        client_board[mine_positions[i][0]][mine_positions[i][1]] = *mine;
    }
}

void play_game() {
    valid_option = false;
    while (!valid_option) {
        drawboard();
        printf("\n%d mines left\n\n", remaining_mines);
        printf("%s", "Choose an option: \n");
        printf("%s", "<R> Reveal tile\n");
        printf("%s", "<F> Place flag\n");
        printf("%s", "<Q> Quit\n\n");
        printf("%s", "Option (R,F,Q): ");
        scanf("%c", &option_input);

        if(option_input == 'r' || option_input == 'f' || option_input == 'q') {
            valid_option = true;
        }
    }

    if (option_input == 'r' || option_input == 'f') {
        printf("\nEnter x coordinate: ");
        scanf("%d", &x_input);
        printf("Enter y coordinate: ");
        scanf("%d", &y_input);
        if (option_input == 'r') {
            int tile_no = check_tile(x_input, y_input);

            if (tile_no == -1) {
                reveal_mines();

                client_board[x_input][y_input] = *revealed;
                drawboard();
                printf("You have revealed a mine. Game over\n\n");
                playing_game = false;
                getchar();
                //display_welcome();
            }

            reveal_tile(x_input, y_input);
            open_safe_tiles(x_input, y_input);
        } else {
            if (server_board[x_input][y_input] == *mine) {
                server_board[x_input][y_input] = *flag;
                client_board[x_input][y_input] = *flag; 
                remaining_mines--;           
            } else if (client_board[x_input][y_input] == *flag) {
                printf("\nYou have already flagged this tile\n");
            } else {
                printf("\nNo mine there try again\n");
            }

            if (remaining_mines == 0) {
                printf("You won\n");
                playing_game = false;
            }
        }

        valid_option = false;
    } else if (option_input == 'q') {
        playing_game = false;
        //display_welcome();
    }
}

int SIDE;
int MINES;

char * serverAddress;
uint16_t serverPort;

int client_socket;
struct sockaddr_in socketAddress;
struct hostent * server;

char username[MAX_USERNAME];
int session = -1;

ClientGameState * gameState = NULL;

int main(int argc, char ** argv) {
    switch (argc) {
        case 1:
            printf("Too few arguments!!\n");
            return 1;
        case 2:
            serverAddress = argv[1];
            serverPort = DEFAULT_PORT;
            break;
        case 3:
            serverAddress = argv[1];
            serverPort = atoi(argv[2]);
            break;
        default:
            printf("Too many arguments!!\n");
            return 1;
    }

    validatePort(serverPort);

    if (serverAddress == NULL) {
        printf("Server address error!\n");
        return 1;
    }

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "Host is unknown!\n");
        exit(0);
    }

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        error("Creating socket failed");
    }

    bzero((char *) &socketAddress, sizeof(socketAddress));
    socketAddress.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &socketAddress.sin_addr.s_addr, (size_t) server->h_length);
    socketAddress.sin_port = htons(serverPort);

    if (connect(client_socket, (const struct sockaddr *) &socketAddress, sizeof(socketAddress)) < 0) {
        error("connecting to server failed");
    }

    drawWelcomeMessage();
    drawScreen(LOGIN);
    DataPacket packet;
    packet.type = CLOSE_CLIENT_PACKET;
    packet.session = session;
    send(client_socket, &packet, sizeof(packet), 0);
    if (gameState != NULL) {
        free(gameState);
    }
}

/**
 * TODO: comment
 */
void drawBorder(int num) {
    for (int i = 0; i < num; i++) {
        printf("==");
    }
}

/**
 * TODO: comment
 */
void drawWelcomeMessage() {
    int borderNum = 24;
    drawBorder(borderNum);
    printf("\n\nWelcome to the online Minesweeper gaming system\n\n");
    drawBorder(borderNum);
    printf("\n");
}

/**
 * TODO: commnet
 */
void SetDrawLoginScreen() {
    printf("\n\n\nYou are required to logon with your registered Username and Password\n\n");
    printf("Username -> ");
    scanf("%s", username);
    printf("Password -> ");
    char password[MAX_PASSWORD];
    scanf("%s", password);
    printf("\n");

    if (isAuthenticateUser(username, password)) {
        drawScreen(MENU);
    } else {
        printf("Either an unauthorized username or password\n");
        exit(1);
    }
}

/**
 * TODO: comment
 */
void SetDrawMenuScreen() {

    int status = 0;
    int choice = 0;

    while (status == 0 || choice < 1 || choice > 3) {
        printf("\nWelcome to the Minesweeper gaming system.\n\n");
        printf("Please enter a selection\n");
        printf("<1> Play Minesweeper\n");
        printf("<2> Show Leaderboard\n");
        printf("<3> Quit\n\n");
        printf("Selection option (1-3): ");

        status = scanf("%d", &choice);

        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }

    switch (choice) {
        case 1:
            initialise_client_board();
            place_mines();
            playing_game = true;
            drawScreen(GAME);
            break;
        case 2:
            drawScreen(LEADERBOARD);
            break;
        case 3:
            return;
        default:
            drawScreen(MENU);
            break;
    }
}

/**
 * TODO: comment
 */
void SetDrawWinScreen() {
    printf("Congratulations! You have located all the mines.\n\n");
    drawScreen(MENU);
}

/**
 * TODO: comment
 */
void SetDrawLoseScreen() {
    printf("\nGame over! You hit a mine\n\n");
    drawScreen(MENU);
}

/**
 * TODO: comment
 */
int SetDrawLeaderboardSegment() {
    int borderNum = 38;
    DataPacket inputPacket;
    recv(client_socket, &inputPacket, sizeof(DataPacket), 0);

    if (inputPacket.type == END_LEADERBOARD_PACKET) {
        return -1;
    } else if (inputPacket.type == ENTRY_LEADERBOARD_PACKET) {
        LeaderboardEntry leaderBoard;
        recv(client_socket, &leaderBoard, sizeof(LeaderboardEntry), 0);

        printf("\n");
        drawBorder(borderNum);

        printf("\n\nPlayer - %s", leaderBoard.username);
        printf("\nNumber of games won - %d", leaderBoard.wins);
        printf("\nNumber of games played - %d\n\n", leaderBoard.games);

        drawBorder(borderNum);
        printf("\n");
    } else {
        error("Unexpected packet!");
        return -1;
    }

    return 0;
}

/**
 * TODO: comment
 */
void SetDrawLeaderboardScreen() {
    int borderNum = 38;
    DataPacket packet;
    packet.type = LEADERBOARD_PACKET;
    packet.session = session;

    send(client_socket, &packet, sizeof(packet), 0);

    int drawnEntries = 0;

    DataPacket inputPacket;
    recv(client_socket, &inputPacket, sizeof(DataPacket), 0);

    if (inputPacket.type != START_LEADERBOARD_PACKET) {
        error("Receiving weird packet!");
        return;
    }

    while (true) {
        if (SetDrawLeaderboardSegment() == -1) {
            break;
        } else {
            drawnEntries++;
        }
    }

    if (drawnEntries == 0) {
        printf("\n");
        drawBorder(borderNum);
        printf("\nThere is no information currently stored in the Leader Board. Try again later\n");
        drawBorder(borderNum);
        printf("\n\n");
    }
    drawScreen(MENU);
}

/**
 * TODO: comment
 */
void SetDrawGameScreen() {
    gameStart();
    srand(RANDOM_NUMBER_SEED);
    while (playing_game){
        play_game();
    }
}

/**
 * TODO: comment
 */
void drawScreen(ScreenType screenType) {
    switch (screenType) {
        case LOGIN:
            SetDrawLoginScreen();
            break;
        case MENU:
            SetDrawMenuScreen();
            break;
        case LOSE:
            SetDrawLoseScreen();
            break;
        case WIN:
            SetDrawWinScreen();
            break;
        case LEADERBOARD:
            SetDrawLeaderboardScreen();
            break;
        case GAME:
            SetDrawGameScreen();
            break;
    }
}

/**
 * TODO: comment
 */
bool isAuthenticateUser(char username[], char password[]) {
    LoginDetails payload;
    strcpy(payload.username, username);
    strcpy(payload.password, password);

    DataPacket dataPacket;
    dataPacket.type = LOGIN_PACKET;

    send(client_socket, &dataPacket, sizeof(dataPacket), 0);
    send(client_socket, &payload, sizeof(LoginDetails), 0);

    DataPacket inputPacket;
    recv(client_socket, &inputPacket, sizeof(DataPacket), 0);

    if (inputPacket.type != LOGIN_RESPONSE_PACKET) {
        printf("Got packet %d.", inputPacket.type);
        error("Received wrong packet. Login response packet expected");
    }
    session = inputPacket.session;

    LoginResponse loginResponse;
    recv(client_socket, &loginResponse, sizeof(LoginResponse), 0);
    bool success = loginResponse.success;

    return success;
}

/**
 * TODO: comment
 */
void SetReceiveGameState() {
    DataPacket inputPacket;
    recv(client_socket, &inputPacket, sizeof(DataPacket), 0);

    if (inputPacket.type != STATE_RESPONSE_PACKET) {
        if (inputPacket.type == INVALID_MINESWEEPER_PACKET) {
            printf("\n\nGame over\n\n");
            return;
        } else {
            printf("Got packet %d.", inputPacket.type);
            error("Received wrong packet. State response packet expected");
        }
    }

    if (gameState == NULL) {
        gameState = malloc(sizeof(ClientGameState));
    }
    recv(client_socket, gameState, sizeof(ClientGameState), 0);
}

/**
 * TODO: comment
 */
void gameStart() {
    DataPacket packet;
    packet.type = START_PACKET;
    packet.session = session;
    send(client_socket, &packet, sizeof(packet), 0);
    SetReceiveGameState();
}
