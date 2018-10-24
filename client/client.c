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

#define DIFFICULTY 0
#define MAXSIDE 25
#define MAXMINES 99
#define MOVESIZE 526

///////////////////////////////////////////////////////////////////

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
    //int x, y;
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

int sockfd;
struct sockaddr_in socketAddress;
struct hostent * server;

char username[USERNAME_MAX_LENGTH];
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
        printf("Provide a server address!\n");
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("Failed to create socket");
    }

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "Unknown host\n");
        exit(0);
    }

    bzero((char *) &socketAddress, sizeof(socketAddress));
    socketAddress.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &socketAddress.sin_addr.s_addr, (size_t) server->h_length);
    socketAddress.sin_port = htons(serverPort);

    if (connect(sockfd, (const struct sockaddr *) &socketAddress, sizeof(socketAddress)) < 0) {
        error("Failed to connect to server");
    }

    drawWelcomeText();

    drawScreen(LOGIN);

    DataPacket packet;
    packet.type = CLOSE_CLIENT_PACKET;
    packet.session = session;

    send(sockfd, &packet, sizeof(packet), 0);

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
void drawWelcomeText() {
    int borderNum = 24;
    drawBorder(borderNum);
    printf("\n\nWelcome to the online Minesweeper gaming system\n\n");
    drawBorder(borderNum);
    printf("\n");
}

/**
 * TODO: commnet
 */
void _drawLoginScreen() {
    printf("\n\n\nYou are required to logon with your registered Username and Password\n\n");
    printf("Username -> ");
    scanf("%s", username);
    printf("Password -> ");
    char password[PASSWORD_MAX_LENGTH];
    scanf("%s", password);
    printf("\n");

    if (authenticateUser(username, password)) {
        drawScreen(MENU);
    } else {
        printf("Either an unauthorized username or password\n");
        exit(1);
    }
}

/**
 * TODO: comment
 */
void _drawMenuScreen() {

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
void _drawWinScreen() {
    printf("Congratulations! You have located all the mines.\n\n");
    drawScreen(MENU);
}

/**
 * TODO: comment
 */
void _drawGameOverScreen() {
    printf("\nGame over! You hit a mine\n\n");
    drawScreen(MENU);
}

/**
 * TODO: comment
 */
int _displayLeaderboardSegment() {
    int borderNum = 38;
    DataPacket inputPacket;
    recv(sockfd, &inputPacket, sizeof(DataPacket), 0);

    if (inputPacket.type == END_LEADERBOARD_PACKET) {
        return -1;
    } else if (inputPacket.type == ENTRY_LEADERBOARD_PACKET) {
        LeaderboardEntry entry;
        recv(sockfd, &entry, sizeof(LeaderboardEntry), 0);

        printf("\n");
        drawBorder(borderNum);

        printf("\n\nPlayer - %s", entry.username);
        printf("\nNumber of games won - %d", entry.wins);
        printf("\nNumber of games played - %d\n\n", entry.games);

        drawBorder(borderNum);
        printf("\n");
    } else {
        error("Received unexpected packet!");
        return -1;
    }

    return 0;
}

/**
 * TODO: comment
 */
void _drawLeaderboardScreen() {
    int borderNum = 38;
    DataPacket packet;
    packet.type = LEADERBOARD_PACKET;
    packet.session = session;

    send(sockfd, &packet, sizeof(packet), 0);

    int drawnEntries = 0;

    DataPacket inputPacket;
    recv(sockfd, &inputPacket, sizeof(DataPacket), 0);

    if (inputPacket.type != START_LEADERBOARD_PACKET) {
        error("Received unexpected packet!");
        return;
    }

    while (true) {
        if (_displayLeaderboardSegment() == -1) {
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
void _drawGameScreen() {
    gameStart();
    srand(RANDOM_NUMBER_SEED);
    while (playing_game){
        play_game();
    }
    /*while (gameState->remainingMines > 0 && !gameState->won) {
        printf("\n");
        for (int i = 0; i < 20; i++) {
            printf("-");
        }
        printf("\n\n\n");
        printf("Guessed letters: %s\n\n", gameState->guessedLetters);
        printf("Number of guesses left: %d\n\n", gameState->remainingMines);
        printf("Word: %s\n\n", gameState->currentGuess);
        printf("Enter your guess -> ");
        char choice;
        int status = scanf("%c", &choice);
        if (status == 0) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        } else {
            guessCharacter(choice);
        }
    }
    Tile* board[height][width];
    for (int i = 0; i < height; i++){
      for (int j = 0; j < width; j++){
        board[i][j] = createTile(i, j);
      }
    }

    cursor = createCursor(height, width);

    srand(time(NULL));

    while(play){
      if (!firstMove){
        printMineInfo();
      }
      printBoard(board);
      if (scanf(" %c", &currentCommand) != -1){
        readAndExecuteInput(board);
        winCheck(board);
      } else {
        printf("Invalid entry.\n");
      }

      if (win || lose){
        printBoard(board);
        if (win){
          printf("Congratulations, you won!\n");
        } else if (lose){
          printf("Oh, a mine blew up.\n");
        }
        revertToStartingState(board);
        promptNextAction();
      }
    }
    for (int i = 0; i < height; i++){
      for (int j = 0; j < width; j++){
        free(board[i][j]);
      }
    }
    free(cursor);
    printf("Program close.\n");

    //drawScreen(gameState->won ? WIN_SCREEN : GAME_OVER_SCREEN);*/
}

/**
 * TODO: comment
 */
void drawScreen(ScreenType screenType) {
    switch (screenType) {
        case LOGIN:
            _drawLoginScreen();
            break;
        case MENU:
            _drawMenuScreen();
            break;
        case GAMEOVER:
            _drawGameOverScreen();
            break;
        case WIN:
            _drawWinScreen();
            break;
        case LEADERBOARD:
            _drawLeaderboardScreen();
            break;
        case GAME:
            _drawGameScreen();
            break;
    }
}

/**
 * TODO: comment
 */
bool authenticateUser(char username[], char password[]) {
    LoginDetailsPayload payload;
    strcpy(payload.username, username);
    strcpy(payload.password, password);

    DataPacket dataPacket;
    dataPacket.type = LOGIN_PACKET;

    send(sockfd, &dataPacket, sizeof(dataPacket), 0);
    send(sockfd, &payload, sizeof(LoginDetailsPayload), 0);

    DataPacket inputPacket;
    recv(sockfd, &inputPacket, sizeof(DataPacket), 0);

    if (inputPacket.type != LOGIN_RESPONSE_PACKET) {
        printf("Got packet %d.", inputPacket.type);
        error("Received wrong packet. Login response packet expected");
    }
    session = inputPacket.session;

    LoginResponsePayload responsePayload;
    recv(sockfd, &responsePayload, sizeof(LoginResponsePayload), 0);
    bool success = responsePayload.success;

    return success;
}

/**
 * TODO: comment
 */
void _receiveGameState() {
    DataPacket inputPacket;
    recv(sockfd, &inputPacket, sizeof(DataPacket), 0);

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
    recv(sockfd, gameState, sizeof(ClientGameState), 0);
}

/**
 * TODO: comment
 */
void gameStart() {
    DataPacket packet;
    packet.type = START_PACKET;
    packet.session = session;

    send(sockfd, &packet, sizeof(packet), 0);

    _receiveGameState();
}
