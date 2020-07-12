#ifndef COMMS_H
#define COMMS_H

#define CMDLEN 1024 // The length of the buffer
#define WAIT 1.5 // How many seconds to wait before calling recv()

int comms_fd(int fd_new);
char *send_cmd();
void get_output();
void send_file(char *file);
void get_file(char *file);
void screenshot();

#endif
