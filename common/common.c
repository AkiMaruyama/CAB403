#include "common.h"

#include <stdio.h>
#include <stdlib.h>

/*
 * this function checks if the current port number is valid.
 * if it's valid, this function will return 0.
 * otherwise returns 1.
 */
int validatePort(int port) {
    int validPort = 65535;
    if (port <= 0 || port >= validPort) {
        printf("Invalid port, %d. Ports must be between 1 and %d", port, validPort);
        return 1;
    }
    return 0;
}

/*
 * this function displays the error message on a screen.
 * then exits the program.
 */
void error(char * msg) {
    perror(msg);
    exit(1);
}
