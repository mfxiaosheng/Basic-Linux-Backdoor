#ifndef COMMS_H
#define COMMS_H

#define CMDLEN 1024

int comms_fd(int fd_new);
char *get_cmd();
void send_output(char *out);
void send_file(char *file);
void get_file(char *file);

#endif
