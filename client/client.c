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

////////////////////////////////////////////////////////////////////
#define BUFF_SIZE 50
#define BASE 10
#define width 10
#define height 10

double difficulty = .10;

int numMines = 0;
int minesMarked = 0;

bool play = true;
bool firstMove = true;

bool win = false;
bool lose = false;

char currentCommand = 'h';

typedef struct _cursor {
  int x;
  int y;
} Cursor;

typedef struct _tile {
  int x;
  int y;
  int surroundingMines;
  bool hasMine;
  bool isHidden;
  bool markedAsMine;
} Tile;

Cursor* createCursor(int boardHeight, int boardWidth){
  Cursor* c = (Cursor*) malloc(sizeof(Cursor));
  c->x = boardHeight/2;
  c->y = boardWidth/2;
  return c;
}

void moveCursor(Cursor* c, int xIncr, int yIncr, int boardLength, int boardWidth){
  if (c->x + xIncr >= boardLength){
    c->x = 0;
  } else if (c->x + xIncr < 0){
    c->x= boardLength - 1;
  } else {
    c->x += xIncr;
  }
  if (c->y + yIncr >= boardWidth){
    c->y = 0;
  } else if (c->y + yIncr < 0){
    c->y = boardWidth - 1;
  } else {
    c->y += yIncr;
  }
}

void printCursor(Cursor* c, bool isHidden, bool markedAsMine, int surroundingMines, bool lose, bool win){
  if (lose){
    printf("L ");
    return;
  } else if (win){
    printf("W ");
    return;
  }

  if (markedAsMine){
    printf("X ");
  } else {
    printf("C ");
  }
}

Tile* createTile(int xCoord, int yCoord){
  Tile* t = (Tile*) malloc(sizeof(Tile*));
  t->isHidden = true;
  t->hasMine = false;
  t->markedAsMine = false;
  t->surroundingMines = 0;
  t->x = xCoord;
  t->y = yCoord;
  return t;
}

void printTile(Tile* t, bool lose, bool win){
  if (lose){
    if (t->hasMine){
      if (t->markedAsMine){
	printf("F ");
      } else {
	printf("B ");
      }
      return;
    }
  } else if (win){
    if (t->hasMine){
      printf("X ");
      return;
    }
  }
  if (t->markedAsMine){
    printf("X ");
    return;
  }
  if (t->isHidden){
    printf("T ");
  } else {
    if (t->surroundingMines == 0){
      printf("0 ");
    } else{
      printf("%d ", t->surroundingMines);
    }
  }
}

Cursor* cursor;

void printControls(){
  printf("\n");
  printf("Choose an aption:\n");
  printf("<R> Reveal tile\n");
  printf("<P> Place flag\n");
  printf("<Q> Quit game\n");
  printf("\n");
}

void printMineInfo(){
  printf("Remaining mines: %d\n", numMines);
}

void printBoard(Tile* board[height][width]){
  printf("   ");
  for (int i = 0; i < width; i++){
    printf("%d ", i + 1);
  }
  printf("\n");
  for (int i = 0; i < 25; i++){
    printf("-");
  }
  printf("\n");
  for (int i = 0; i < height; i++){
    if (i < 9){
      printf("%d |", i + 1);
    } else {
      printf("%d|", i + 1);
    }
    for (int j = 0; j < width; j++){
      if (i == cursor->x && j == cursor->y){
        printCursor(cursor, board[i][j]->isHidden, board[i][j]->markedAsMine, board[i][j]->surroundingMines, lose, win);
      } else {
	printTile(board[i][j], lose, win);
      }
    }
    printf("\n");
  }
  printf("\n");
}

void markAsMine(Tile* board[height][width]){
  if (!board[cursor->x][cursor->y]->isHidden){
    return;
  }
  if (board[cursor->x][cursor->y]->markedAsMine){
    minesMarked--;
  } else {
    minesMarked++;
  }
  board[cursor->x][cursor->y]->markedAsMine = !board[cursor->x][cursor->y]->markedAsMine;
}

bool inArray(int* arr, int x, int l){
  int in = 0;
  for (int i = 0; i < l; i++){
    if (arr[i] == x){
      in++;
      if (in == 2){
        return true;
      }
    }
  }
  return false;
}

void setMines(Tile* board[height][width], int cursorX, int cursorY) {
    int ideal = (int) (height * width * difficulty);
    
    int* mineX = malloc(sizeof(int) * ideal);
    int* mineY = malloc(sizeof(int) * ideal);
    
    /*Probably not the most optimal solution.*/
    for (int i = 0; i < ideal; i++) {
        int x = rand() % height;
        int y = rand() % width;
        
        while ((x == cursorX && y == cursorY) || inArray(mineX, x, height) || inArray(mineY, y, width)) {
            x = rand() % height;
            y = rand() % width;
        }
        
        mineX[i] = x;
        mineY[i] = y;
    }
    
    /*Place mines.*/
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            bool unique = true;
            for (int k = 0; k < ideal; k++) {
                if (board[i][j]->x == mineX[k] && board[i][j]->y == mineY[k]) {
                    if (unique) {
                        numMines++;
                        unique = false;
                    }
                    board[i][j]->hasMine = true;
                    board[i][j]->surroundingMines = -1; //set to -1 so activateTile doesn't get confused
                    
                }
            }
        }
    }
    
    free(mineX);
    free(mineY);
}

void incrementTilesAroundMine(Tile* board[height][width], int tileX, int tileY) {
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int this_x = tileX + i;
            int this_y = tileY + j;
            
            if (i == 0 && j == 0) {
                continue;
            } else if (this_x < 0 || this_x >= height) {
                continue;
            } else if (this_y < 0 || this_y >= width) {
                continue;
            } else if (board[this_x][this_y]->hasMine == true) {
                continue;
            } else {
                board[this_x][this_y]->surroundingMines += 1;
            }
        }
    }
}

void calculateSurroudingMines(Tile* board[height][width]) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (board[i][j]->hasMine) {
                incrementTilesAroundMine(board, i, j);
            }
        }
    }
}

void activateTile(Tile* board[height][width], int tileX, int tileY, bool initialCall) {
     if (board[tileX][tileY]->markedAsMine || !board[tileX][tileY]->isHidden) {
        return;
    }
    
    if (board[tileX][tileY]->hasMine && initialCall) {
        lose = true;
        return;
    }
    
    if (board[tileX][tileY]->isHidden && !board[tileX][tileY]->hasMine) {
        board[tileX][tileY]->isHidden = false;
        if (board[tileX][tileY]->surroundingMines == 0 || firstMove) {
            firstMove = false;
            for (int i = -1; i <= 1; i++){
                for (int j = -1; j <= 1; j++){
                    int this_x = tileX + i;
                    int this_y = tileY + j;
                    
                    if (i == 0 && j == 0) {
                        continue;
                    } else if(this_x < 0 || this_x >= height) {
                        continue;
                    } else if(this_y < 0 || this_y >= width) {
                        continue;
                    }
                    activateTile(board, tileX+i,tileY+j, false);
                }
            }
        }
    }
}

void activateSurroundingTiles(Tile* board[height][width], int cursorX, int cursorY) {
    if (firstMove || board[cursorX][cursorY]->isHidden || board[cursorX][cursorY]->surroundingMines == 0) {
        return;
    }
    
    int surroundingMarkedAsMine = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int this_x = cursorX + i;
            int this_y = cursorY + j;
            
            if (i == 0 && j == 0) {
                continue;
            } else if (this_x < 0 || this_x >= height) {
                continue;
            } else if (this_y < 0 || this_y >= width) {
                continue;
            } else if (board[this_x][this_y]->markedAsMine) {
                surroundingMarkedAsMine += 1;
            }
        }
    }
    
    if (board[cursorX][cursorY]->surroundingMines != surroundingMarkedAsMine) {
        return;
    }
    
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int this_x = cursorX + i;
            int this_y = cursorY + j;
            
            if (i == 0 && j == 0) {
                continue;
            } else if (this_x < 0 || this_x >= height) {
                continue;
            } else if (this_y < 0 || this_y >= width) {
                continue;
            } else if (board[this_x][this_y]->markedAsMine) {
                continue;
            } else if (board[this_x][this_y]->hasMine) {
                lose = true;
                return;
            } else {
                activateTile(board, this_x, this_y, false);
            }
        }
    }
}

void winCheck(Tile* board[height][width]) {
    bool allUncovered = true;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (!board[i][j]->hasMine && board[i][j]->isHidden) {
                allUncovered = false;
            }
        }
    }
    if (allUncovered) {
        win = true;
    }
}

void revertToStartingState(Tile* board[height][width]) {
    win = false;
    lose = false;
    
    firstMove = true;
    
    minesMarked = 0;
    numMines = 0;
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            free(board[i][j]);
        }
    }
    free(cursor);
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            board[i][j] = createTile(i, j);
        }
    }
    cursor = createCursor(height, width);
}

void readAndExecuteInput(Tile* board[height][width]) {
    switch (currentCommand) {
        case 'a':
            /*move left*/
            moveCursor(cursor, 0, -1, height, width);
            break;
        case 's':
            /*move down*/
            moveCursor(cursor, 1, 0, height, width);
            break;
        case 'd':
            /*move right*/
            moveCursor(cursor, 0, 1, height, width);
            break;
        case 'w':
            /*move up*/
            moveCursor(cursor, -1, 0, height, width);
            break;
        case 'q':
            play = false;
            break;
        case 'p':
            markAsMine(board);
            break;
        case 'h':
            printControls();
            break;
        case 'r':
            if (firstMove) {
                setMines(board, cursor->x, cursor->y);
                calculateSurroudingMines(board);
            }
            activateTile(board, cursor->x, cursor->y, true);
            break;
        default:
            printf("Command not recognized. Press 'h' for a list of commands.\n");
            break;
    }
}

void promptNextAction() {
    play = false;
    return;
}

////////////////////////////////////////////////////////////////////

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

    drawScreen(LOGIN_SCREEN);

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
        drawScreen(MENU_SCREEN);
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
            drawScreen(GAME_SCREEN);
            break;
        case 2:
            drawScreen(LEADERBOARD_SCREEN);
            break;
        case 3:
            return;
        default:
            drawScreen(MENU_SCREEN);
            break;
    }
}

/**
 * TODO: comment
 */
void _drawWinScreen() {
    printf("Congratulations! You have located all the mines.\n\n");
    drawScreen(MENU_SCREEN);
}

/**
 * TODO: comment
 */
void _drawGameOverScreen() {
    printf("\nGame over! You hit a mine\n\n");
    drawScreen(MENU_SCREEN);
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
    drawScreen(MENU_SCREEN);
}

/**
 * TODO: comment
 */
void _drawGameScreen() {
    startGame();
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
    }*/
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

    //drawScreen(gameState->won ? WIN_SCREEN : GAME_OVER_SCREEN);
}

/**
 * TODO: comment
 */
void drawScreen(ScreenType screenType) {
    switch (screenType) {
        case LOGIN_SCREEN:
            _drawLoginScreen();
            break;
        case MENU_SCREEN:
            _drawMenuScreen();
            break;
        case GAME_OVER_SCREEN:
            _drawGameOverScreen();
            break;
        case WIN_SCREEN:
            _drawWinScreen();
            break;
        case LEADERBOARD_SCREEN:
            _drawLeaderboardScreen();
            break;
        case GAME_SCREEN:
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
        if (inputPacket.type == INVALID_GUESS_PACKET) {
            printf("\n\nYou can't guess that, please try again!\n\n");
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
void startGame() {
    DataPacket packet;
    packet.type = START_PACKET;
    packet.session = session;

    send(sockfd, &packet, sizeof(packet), 0);

    _receiveGameState();
}
