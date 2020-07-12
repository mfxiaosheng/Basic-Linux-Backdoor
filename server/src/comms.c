#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "debug.h"
#include "comms.h"

// Used to set the socket's file descriptor for communication.
// If called with a value of zero, this simply returns the current file descriptor used for communication.
int comms_fd(int fd_new) {
    static int fd;
    if(fd_new != 0) {
        fd = fd_new;
    }
    return fd;
}

// Reads a single command and returns it.
// The string that this returns is allocated on the heap and must be free'd.
char *get_cmd() {
    char *cmd;
    int len;

    // Allocate memory
    cmd = malloc(CMDLEN);
    if(cmd == NULL) {
        debug_err("Error in malloc()");
        exit(1);
    }

    // Read the command
    memset(cmd, 0, CMDLEN);
    len = recv(comms_fd(0), cmd, CMDLEN, 0);
    if(len == -1) {
        debug_err("Error in recv()");
        exit(1);
    } else if(len == 0) {
        cmd = NULL;
    } else {
        debug_print("Obtained command: %s\n", cmd);
    }

    return cmd;
}

// Sends the output of a command to the client.
void send_output(char *out) {
    if(send(comms_fd(0), out, strlen(out), 0) < 0) {
        debug_err("Error in send()");
        exit(1);
    }
}

// Uploads a local file to the server
// Triggered whenever the "upload" command is used
void send_file(char *file) {
    FILE *f;
    size_t filesz, i;
    char byte;

    // Open the file
    f = fopen(file, "r");
    if(f != NULL) {
        // Find the length of the file
        fseek(f, 0L, SEEK_END);
        filesz = ftell(f);
        rewind(f);

        // Send the length of the data to the server
        if(send(comms_fd(0), &filesz, sizeof(filesz), 0) < 0) {
            debug_err("Error in send()");
            exit(1);
        }

        // Send the data byte by byte
        for(i = 0; i < filesz; i++) {
            if(fread(&byte, 1, 1, f)) {
                if(send(comms_fd(0), &byte, 1, 0) < 0) { // Send a single byte
                    debug_err("Error in send()");
                    exit(1);
                }
            }
        }

        fclose(f);
    }
}

// Downloads a remote file from the server.
// Triggered whenever the "download" command is used.
void get_file(char *file) {
    FILE *f;
    size_t filesz, i;
    char byte;

    // Open the file
    f = fopen(file, "w+");
    if(f != NULL) {
        printf("Writing downloaded file to %s...\n", file);

        // Get the length of the data
        if(recv(comms_fd(0), &filesz, sizeof(filesz), 0) == -1) {
            debug_err("Error in recv()");
            exit(1);
        }

        // Read the data byte by byte and print out the byte to the file
        for(i = 0; i < filesz; i++) {
            if(recv(comms_fd(0), &byte, 1, 0) == -1) {
                debug_err("Error in recv()");
            }
            fputc(byte, f);
        }

        fclose(f);
    }
}
