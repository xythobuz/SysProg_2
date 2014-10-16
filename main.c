/*
 * client.c
 *
 * Copyright 2014 Christian Högerle, Thomas Buck
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <string.h>

int main(void) {
    pid_t forkResult;
    int stdinPipe[2];
    int stdoutPipe[2];
    char readBuffer[1024];
    int readPos = 0;
    int summary_read = 0;
    char input[1024];

    // Anlegen der Pipes
    if (pipe(stdinPipe) == -1) {
        perror("pipe in");
        return 1;
    }

    if (pipe(stdoutPipe) == -1) {
        perror("pipe out");
        return 1;
    }

    // Erzeugen des Kindprozesses
    forkResult = fork();
    if (forkResult < 0) {
        perror("fork");
        return 1;
    } else if (forkResult == 0) {
        // im Kindprozess
        if(dup2(stdinPipe[0], STDIN_FILENO) == -1) {
            perror("dup2(stdinPipe[0], STDIN_FILENO)");
            return 1;
        }

        if(dup2(stdoutPipe[1], STDOUT_FILENO) == -1) {
            perror("dup2(stdoutPipe[1], STDOUT_FILENO)");
            return 1;
        }

        close(stdinPipe[1]);
        close(stdinPipe[0]);
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);

        execl("waitforme", "waitforme", NULL);
        perror("ecec:");
        return 1;
    } else {
        // im Elternprozess
        close(stdinPipe[0]);
        close(stdoutPipe[1]);

        fgets(input, sizeof(input), stdin);
        if(write(stdinPipe[1], input, strlen(input)) < strlen(input)) {
            close(stdinPipe[1]);
            close(stdoutPipe[0]);
            perror("write");
            return 1;
        }
        close(stdinPipe[1]);

        // Timeout
        fd_set rfds;
        struct timespec ts;
        FD_ZERO(&rfds);
        FD_SET(stdoutPipe[0], &rfds);
        ts.tv_sec = 5; // 5s
        ts.tv_nsec = 0;

        int retval = pselect(stdoutPipe[0] + 1, &rfds, NULL, NULL, &ts, NULL);
        if (retval == -1) {
            close(stdoutPipe[0]);
            perror("pselect()");
        } else if (retval == 0) {
            printf("No data within five seconds \n");
            close(stdoutPipe[0]);
            return 0;
        } else {
            // Daten aus dem Leseende von stdoutPipe von Kindprozess lesen
            while((readPos < (sizeof(readBuffer) - 1)) && (summary_read = read(stdoutPipe[0],
                            readBuffer + readPos, (sizeof(readBuffer) - 1 - readPos))) > 0) {
                readPos += summary_read;
            }
        }

        readBuffer[readPos] = '\0';
        printf("Ausgabe: %s", readBuffer);
        close(stdoutPipe[0]);
    }

    return 0;
}

