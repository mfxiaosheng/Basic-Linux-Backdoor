# Basic-Linux-Backdoor
This is a basic post-exploitation linux backdoor program that I created purely for educational purposes in order to learn more about cybersecurity. This program should never be executed on any production system, nor should it be used for nefarious or illegal purposes. You should never execute this program on an internet-connected device because it will create security issues.

## Introduction
When the backdoor server is executed on a target linux machine, it will create a server that will passively listen for commands. Using the backdoor client program, an attacker could connect to one of these servers and issue several different commands (listed in the Supported Commands section). The server will execute every command that it receives and return the output of the command to the client. If the backdoor server is running as root, then it will always add a bash command to execute itself in /etc/rc.local, which makes sure that the backdoor will continue to be executed on startup if the system reboots. The backdoor will also attempt to disable common linux firewall programs that are running on the target.

## Usage
1. Since this is meant to only be used post-exploitation, make sure that you have write and execute privileges on some directory on your target.
2. Copy the backdoor server (located inside of the server folder) to the target that you wish to install the backdoor on and execute it, preferably with root privileges.
    - Note that while the server can still send and receive commands from the target, certain aspects of the backdoor (such as disabling firewalls or installing scrot in order to take screenshots) may not work properly without root privileges.
3. In order to connect to the target from a remote machine, copy the backdoor client (located inside of the client folder) to the attacking machine and execute it using the IP address and port that the server is running on as arguments (by default, the server is running on port 8080).
4. You should now be able to issue commands to the target machine from the attacking machine.

## Supported Commands
The client (attacker) can issue any of these commands to the server (target):
- !\<command\>: Executes the given command as if it was running on a shell on the server.
- cd \<dir\>: Changes into the given directory.
- pwd \<dir\>: Prints out the current working directory.
- cat \<file\>: Displays the contents of a given file.
- ls \<dir\>: Lists the contents of a directory (uses current directory if none is given).
- dir \<dir\>: Same as ls.
- rm \<file\>: Deletes a given file.
- upload \<local file\> \<remote file\>: Uploads a local file to the remote target.
- download \<remote file\> \<local file\>: Downloads a remote file onto the local client.
- screenshot: Attempts to take a screenshot of the remote target using scrot, but may fail if scrot could not be installed.
- quit: Closes the connection to the target, but leaves the backdoor server running.
- exit: Same as quit.
- uninstall: Uninstalls the backdoor, closes the server, and closes the connection to the target.

Commands are received by the server using the get\_cmd() function in comms.c. As soon as the server receives one of these commands, it will immediately process it using the process\_cmd() function in cmd.c. The server must always return some output to the client after receiving a command by using the send\_output() function in comms.c because the client will be left waiting for output otherwise. A newline character is simply returned if there is no output. Most of the shell commands are implemented by calling the popen() function, but there are a few commands (such as the upload command or the screenshot command) that require additional processing. The upload, download, and screenshot commands make use of the send\_file() and get\_file() functions, which are located in comms.c, to send files to and from the server.

## Disabling Firewalls
In Linux, most systems will have one of these three firewall programs: firewalld, ufw, and iptables. The disable\_firewalls() function, located in security.c on the backdoor server, will attempt to disable each of these firewalls by executing the following bash commands:
```
systemctl disable firewalld
ufw disable
systemctl disable ufw
service iptables stop
chkconfig iptables off
```
These commands are only executed if the server is running with root privileges. While this will work on most modern linux systems, the downside to using hardcoded bash commands like these is that they will not work on every single system. If a system is running a firewall other than firewalld, ufw, or iptables, then this backdoor will not be able to bring it down (unless the attacker is able to issue a command that brings it down).

## Enabling Persistence
If running with root privileges, The backdoor server will attempt to create a file in the target's root directory called /.update. If the server is successful in creating this file, then it will copy itself onto the /.update file and mark the file as executable. Finally, in order to actually make sure that the /.update file runs when the machine starts up, a line will be added to the /etc/rc.local file (which is always executed on startup) to execute the /.update file in the background. If the "uninstall" command is ever used, then the backdoor server will delete the /.update file and stop the server.

## Uploading/Downloading Files
### Uploading
The attacker can upload local files to the remote target by issuing the upload command. When the client program sees that the attacker has typed in this command, the client will call the send\_file() function in comms.c. This function will first calculate the length of the file in bytes by using the fseek() function, and it will send the length of the file to the server. Then it will loop through every byte in the file and send it to the server. On the server's end, as soon as the server sees that the client has issued the upload command, it will call the get\_file() function in comms.c. This function will create a new file, receive the length of the file, and read each byte from the client in a loop until it has read all of the bytes.

### Downloading
Downloading is essentially the opposite of uploading, so the function calls are switched. Upon seeing that the attacker has issued the download command, the client backdoor program will call the get\_file() function, whereas the server will call the send\_file() function. Everything aside from that is essentially the same as uploading.

## Taking Screenshots
The screenshot command is dependent on the scrot program, which is an application used to take screenshots and is sometimes preinstalled in some Linux distributions, and as a result, this command will not work on every single system. When the server sees that the attacker has issued the screenshot command, the server will attempt to download the scrot command from the internet using both apt-get and yum. The server will then execute the scrot program in order to take a screenshot and save it in the /tmp/ directory. After using executing scrot, if the screenshot does not exist in the /tmp/ directory, then that either means that scrot could not be installed, or that it was not already installed on the target (this could happen if the server isn't running with root privileges or if the server could not connect to the package repositories). If the screenshot does exist in the /tmp/ directory, then the server will call send\_file() to send the file to the client (similar to how the download command works).

## Installation on Flash Drives
This backdoor can easily be installed on a flash drive in order to make a trojan horse. In order to do this, copy the backdoor server application to the flash drive and name it something that seems innocuous, such as "update". Place some basic information about the program that the trojan horse is trying to masquerade as inside of a file called "Autorun.inf", and place your code in a shell script called "autorun.sh". You can also add an icon to the flash drive to make it seem less suspicious. Here is an example where I create a trojan horse that claims that it is "installing updates" while it is actually running the backdoor server in the background.

Autorun.inf:
```
[autorun]
Open=autorun.sh
Action=Start Updates
Label=Updater
Icon=icon.png
```

autorun.sh
```
#!/bin/bash
cd "$(dirname "$0")"
./update | ./progress --progress --pulsate --no-cancel --text="Updating..."
```

Note that the "update" program is actually the backdoor, and the "progress" program is actually a copy of /usr/bin/zenity, which is used to display progress bars. When the flash drive is inserted into a victim's machine, a popup will appear asking it to run the program in the flash drive. If the victim allows the program to run, another popup that says "Updating..." with a pulsating progress bar will appear while the backdoor server runs in the background.
