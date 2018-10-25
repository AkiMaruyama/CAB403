#ifndef __APPLE__
#define _GNU_SOURCE
#endif

#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "map.h"
#include "list.h"
#include "../common/common.h"

/*
 * IMPORTANT VARIABLES AFTER STEP 2
 * DON'T DELETE
 */
Map accounts;
Map scores;
List currentSessions;
List gameSessions;

uint16_t serverPort;
int sockfd, new_fd;
volatile int highestSession = 0;

#ifdef __APPLE__
/* the pthread mutex initializer for APPLE products  */
pthread_mutex_t request_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER;
pthread_mutex_t scores_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER;
#else
/* the pthread mutex initializer for NON_APPLE products */
pthread_mutex_t request_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t scores_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
#endif

pthread_cond_t got_request = PTHREAD_COND_INITIALIZER;

int num_requests = 0;
struct request * requests = NULL;
struct request * last_request = NULL;

/*
 * this function add requests to memory
 */
void add_request(int socket_id, pthread_mutex_t * p_mutex, pthread_cond_t * p_cond_var) {
    struct request * newRequest = (struct request *) malloc(sizeof(struct request));
    if (!newRequest) {
        error("MEMORY IS OUT!");
        return;
    }
    newRequest->socket_id = socket_id;
    newRequest->next = NULL;

    pthread_mutex_lock(p_mutex);

    if (num_requests == 0) {
        requests = newRequest;
        last_request = newRequest;
    } else {
        last_request->next = newRequest;
        last_request = newRequest;
    }

    num_requests++;

    pthread_mutex_unlock(p_mutex);
    pthread_cond_signal(p_cond_var);
}

/*
 * this structure controls the numbers of requests
 */
struct request * get_request(pthread_mutex_t * p_mutex) {
    struct request * a_request;

    pthread_mutex_lock(p_mutex);

    if (num_requests > 0) {
        a_request = requests;
        requests = a_request->next;
        if (requests == NULL) {
            last_request = NULL;
        }

        num_requests--;
    } else {
        a_request = NULL;
    }

    pthread_mutex_unlock(p_mutex);

    return a_request;
}

/*
 * this function handles the responses from the client side.
 * also, controls the requests of pthead
 */
void * handleResponseLoop(void * data) {
    struct request * a_request;
    int thread_id = *((int *) data);

    pthread_mutex_lock(&request_mutex);

    while (1) {
        if (num_requests > 0) {
            a_request = get_request(&request_mutex);
            if (a_request) {
                pthread_mutex_unlock(&request_mutex);
                handleResponse(a_request, thread_id);
                free(a_request);
                pthread_mutex_lock(&request_mutex);
            }
        } else {
            pthread_cond_wait(&got_request, &request_mutex);
        }
    }
}

int main(int argc, char ** argv) {
    struct sockaddr_in my_address;
    struct sockaddr_in their_address;
    socklen_t sin_size;
    signal(SIGINT, interruptHandler);
    signal(SIGHUP, interruptHandler);

    /*
     * controls port numbers
     */
    switch (argc) {
        case 1:
            serverPort = DEFAULT_PORT;
            break;
        case 2:
            serverPort = atoi(argv[1]);
            break;
        default:
            error("Too many arguments!!");
            return -1;
    }
    validatePort(serverPort);

    if (loadAccounts() != 0) {
        error("ACCOUNT LOAD FAIL");
        return -1;
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        error("SOCKET CREATION FAIL");
        return -1;
    }

    my_address.sin_family = AF_INET;
    my_address.sin_port = htons(serverPort);
    my_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *) &my_address, sizeof(struct sockaddr)) == -1) {
        error("PORT BIND FAIL");
        return -1;
    }

    if (listen(sockfd, BACKLOGIN) == -1) {
        error("SOCKET FAIL");
        return -1;
    }

    currentSessions = createList(4, sizeof(SessionHandle));
    gameSessions = createList(4, sizeof(ServerGameState));
    scores = createMap(4);

    printf("Server is on port %d \n", serverPort);


    int threadIds[BACKLOGIN];
    pthread_t p_threads[BACKLOGIN];

    for (int i = 0; i < BACKLOGIN; i++) {
        threadIds[i] = i;
        pthread_create(&p_threads[i], NULL, handleResponseLoop, (void *) &threadIds[i]);
    }

    while (1) {
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sockfd, (struct sockaddr *) &their_address, &sin_size)) == -1) {
            perror("Failed to accept a connection");
            continue;
        }
        printf("Got a connection from: %s\n", inet_ntoa(their_address.sin_addr));
        add_request(new_fd, &request_mutex, &got_request);
    }

    finish();
    return 0;
}

/*
 * this function get the index of state by the current session
 */
int getStateIndexBySession(int session) {
    for (int i = 0; i < gameSessions->length; i++) {
        ServerGameState * serverGameState = ((ServerGameState *) getValueAt(gameSessions, i));
        if (serverGameState == NULL) {
            error("Encountered null gamestate!");
            return -1;
        }
        if (serverGameState->session == session) {
            return i;
        }
    }
    return -1;
}

/*
 * this function get state by the current session
 */
ServerGameState * getStateBySession(int session) {
    int index = getStateIndexBySession(session);
    return index != -1 ? getValueAt(gameSessions, index) : NULL;
}

/*
 * this function handles packets for login, start, minesweeper, leaderboard, and close_client
 */
void * handleResponse(struct request * a_request, int thread_id) {
    int sock = a_request->socket_id;
    DataPacket inputPacket;

    while (recv(sock, &inputPacket, sizeof(DataPacket), 0) > 0) {
        SessionHandle * currentSession = NULL;
        if (inputPacket.session != -1) {
            for (int i = 0; i < currentSessions->length; i++) {
                SessionHandle * sessionHandle = getValueAt(currentSessions, i);
                if (sessionHandle->session == inputPacket.session) {
                    currentSession = sessionHandle;
                    break;
                }
            }
        }

        switch (inputPacket.type) {
            /* controls the login packet */
            case LOGIN_PACKET: {
                LoginDetailsPayload detailsPayload;
                recv(sock, &detailsPayload, sizeof(LoginDetailsPayload), 0);

                char username[USERNAME_MAX_LENGTH], password[PASSWORD_MAX_LENGTH];
                strcpy(username, detailsPayload.username);
                strcpy(password, detailsPayload.password);

                bool response = false;
                if (containsEntry(accounts, username)) {
                    if (strcmp(getValue(accounts, username), password) == 0) {
                        response = true;
                    }
                }

                LoginResponsePayload loginResponse;
                loginResponse.success = response;

                DataPacket packet;
                packet.type = LOGIN_RESPONSE_PACKET;
                packet.session = response ? (highestSession++) : -1;

                if (response) {
                    SessionHandle * store = malloc(sizeof(SessionHandle));
                    strcpy(store->username, username);
                    store->session = packet.session;
                    add(currentSessions, store);
                }

                send(sock, &packet, sizeof(DataPacket), 0);
                send(sock, &loginResponse, sizeof(LoginResponsePayload), 0);
                break;
            }
            /* controls the start packet */
            case START_PACKET: {
                if (currentSession == NULL) {
                    printf("Non-logged in user tried to start a game!");
                    break;
                }

                ServerGameState serverState;
                serverState.session = inputPacket.session;
                add(gameSessions, &serverState);

                ClientGameState state;
                state.won = false;

                DataPacket packet;
                packet.type = STATE_RESPONSE_PACKET;
                packet.session = inputPacket.session;

                send(sock, &packet, sizeof(DataPacket), 0);
                send(sock, &state, sizeof(ClientGameState), 0);
                break;
            }
            /* controls the minesweeper packet */
            case MINESWEEPER_PACKET: {
                if (currentSession == NULL) {
                    printf("Session Error");
                    break;
                }

                TakeTurnPayload takeTurnPayload;
                recv(sock, &takeTurnPayload, sizeof(TakeTurnPayload), 0);

                ServerGameState * serverState = getStateBySession(inputPacket.session);
                if (serverState == NULL) {
                    printf("Invalid session %d", inputPacket.session);
                    break;
                }

                ClientGameState state;

		state.won = 0;

                if (state.won) {
                    LeaderboardEntry * entry = getScoreForPlayer(currentSession->username);
                    entry->games++;
                    strcpy(entry->username, currentSession->username);
                    if (state.won) {
                        entry->wins++;
                    }
                }

                DataPacket packet;
                packet.type = STATE_RESPONSE_PACKET;
                packet.session = inputPacket.session;

                send(sock, &packet, sizeof(DataPacket), 0);
                send(sock, &state, sizeof(ClientGameState), 0);
                break;
            }
            /* controls the leaderboard packet */
            case LEADERBOARD_PACKET: {
                if (currentSession == NULL) {
                    printf("Non-logged in user tried to request leaderboard!");
                    break;
                }
                DataPacket packet;
                packet.type = START_LEADERBOARD_PACKET;
                packet.session = inputPacket.session;

                send(sock, &packet, sizeof(DataPacket), 0);

                int length;
                pthread_mutex_lock(&scores_mutex);
                LeaderboardEntry ** leaderboardArray = (LeaderboardEntry **) getValues(scores, sizeof(LeaderboardEntry),
                                                                                       &length);
                pthread_mutex_unlock(&scores_mutex);

                if (length > 1) {
                    for (int i = length - 1; i > 0; i--) {
                        for (int j = 0; j < i; ++j) {
                            LeaderboardEntry * a = leaderboardArray[j];
                            LeaderboardEntry * b = leaderboardArray[j + 1];
                            if (a->wins > b->wins
                                || (a->wins == b->wins && (a->wins / (float) a->games) > (b->wins / (float) b->games))
                                || strcmp(a->username, b->username) > 0) {
                                leaderboardArray[j + 1] = a;
                                leaderboardArray[j] = b;
                            }
                        }
                    }
                }

                for (int i = 0; i < length; i++) {
                    LeaderboardEntry * leaderboardEntry = leaderboardArray[i];

                    packet.type = ENTRY_LEADERBOARD_PACKET;
                    packet.session = inputPacket.session;

                    send(sock, &packet, sizeof(DataPacket), 0);
                    send(sock, leaderboardEntry, sizeof(LeaderboardEntry), 0);
                }

                packet.type = END_LEADERBOARD_PACKET;
                packet.session = inputPacket.session;

                send(sock, &packet, sizeof(DataPacket), 0);
                break;
            }
            /* controls the close client packet */
            case CLOSE_CLIENT_PACKET: {
                if (inputPacket.session != -1) {
                    removeAt(gameSessions, getStateIndexBySession(inputPacket.session));
                    for (int i = 0; i < currentSessions->length; i++) {
                        SessionHandle * sessionHandle = getValueAt(currentSessions, i);
                        if (sessionHandle->session == inputPacket.session) {
                            removeAt(currentSessions, i);
                            break;
                        }
                    }
                    return NULL;
                }
                break;
            }
            default:
                perror("Unknown packet type");
                break;
        }
    }

    return NULL;
}

/*
 * this function gets the scores for each player
 */
LeaderboardEntry * getScoreForPlayer(char username[USERNAME_MAX_LENGTH]) {
    pthread_mutex_lock(&scores_mutex);
    LeaderboardEntry * entry = getValue(scores, username);
    pthread_mutex_unlock(&scores_mutex);
    if (entry == NULL) {
        entry = malloc(sizeof(LeaderboardEntry));
        entry->games = 0;
        entry->wins = 0;
        pthread_mutex_lock(&scores_mutex);
        putEntry(scores, username, entry);
        pthread_mutex_unlock(&scores_mutex);
    }

    return entry;
}

/*
 * this function loads the users account
 */
int loadAccounts() {
    accounts = createMap(16);

    FILE * auth_file_handle = fopen("Authentication.txt", "r");
    if (auth_file_handle == NULL) {
        printf("Failed to open Authentication.txt! Does it exist?\n");
        return 1;
    }

    size_t len = 0;
    char * line = NULL;
    int lineNum = -1;

    while ((getline(&line, &len, auth_file_handle)) != -1) {
        lineNum++;

        if (lineNum == 0) {
            continue;
        }
        char * token = strtok(line, "\t\n\r ");
        int tokenNumber = 0;

        char * username = NULL;
        char * password = NULL;

        while (token != NULL) {
            if (tokenNumber == 0) {
                username = malloc(strlen(token) * sizeof(char));
                strcpy(username, token);
            } else if (tokenNumber == 1) {
                password = malloc(strlen(token) * sizeof(char));
                strcpy(password, token);
            }
            token = strtok(NULL, "\t\n\r ");
            tokenNumber++;
        }

        if (username == NULL || password == NULL) {
            printf("Invalid username and password on line %d.\n", lineNum + 1);
            fclose(auth_file_handle);
            return 1;
        }
        putEntry(accounts, username, password);
    }

    fclose(auth_file_handle);

    return 0;
}

/*
 * this function controls the interrupt signal to close program
 */
void interruptHandler(int signal) {
    if (signal == SIGINT || signal == SIGHUP) {
        printf("\nShut down\n");
        finish();
        exit(0);
    }
}

/*
 * this function releases all memory captured
 */
void finish() {
    freeMap(accounts);
    freeList(gameSessions);
    freeList(currentSessions);
    freeMap(scores);
    shutdown(sockfd, SHUT_RDWR);
}
