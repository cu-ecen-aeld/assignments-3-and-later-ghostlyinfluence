#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// struct addrinfo {
//     int     ai_flags;
//     int     ai_family;
//     int     ai_socktype;
//     int     ai_protocol;
//     socklen_t   ai_addrlen;
//     struct sockaddr *ai_addr;
//     char    *ai_canonname;
//     struct addrinfo *ai_next;
// };

int main(int argc, char *argv[]) {
    
    // DONE: Logs message to the syslog
    openlog(NULL, 0, LOG_USER);
    syslog(LOG_INFO, "syslog opened for aesdsocket");

    // TODO: Add arg parsing for -d "Daemon command"

    // TODO: Handle SIGINT and SIGTERM being received

    // Follow what the video said to do. ALSO, the getaddrinfo man page has a great example
    struct addrinfo hints;
    struct addrinfo *servinfo, *rp;
    int status;
    int sockfd;
    
    //Setup the socket 'hints'
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_addrlen = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;

    // getaddrinfo step
    if ((status = getaddrinfo(NULL, 9000, &hints, &servinfo)) != 0) {
        syslog(LOG_ERR, "getaddrinfo: %s\n", gai_strerror(status));
        return -1;
    }

    // open socket step
    for (rp = servinfo; rp != NULL; rp = rp->ai_next) {
        sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
        if (sockfd == -1) {
            continue;
        }

        if (bind(sockfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            break;
        }
        close(sockfd);
    }

    freeaddrinfo(servinfo);

    // Verify that it could bind
    if (rp == NULL) {
        syslog(LOG_ERR, "could not bind");
        return -1;
    }

    int connfd;
    int backlog = 128; //Number from manpage

    // Create directory and file
    syslog(LOG_INFO, "Creating /var/tmp/aesdsocketdata if not already created");
    const char cmd_str[] = "mkdir -p /var/tmp; touch /var/tmp/aesdsocketdata";
    char tcmd[sizeof(cmd_str) + 1];
    sprintf(tcmd, cmd_str);
    return system(tcmd);

    syslog(LOG_INFO, "Server starter: entering main loop");
    // Main loop for listening, accepting, and handling connections
    while(1) {
        // listen for new client
        if ((listen(sockfd, backlog)) == -1) {
            syslog(LOG_ERR, "listen failed");
            close(sockfd);
            return -1;
        }

        // accept connection
        struct sockaddr_storage clientaddr;
        socklen_t clientaddrlen = sizeof(clientaddr);
        connfd = accept(sockfd, (struct sockaddr *)&clientaddr, clientaddrlen);
        if (connfd == -1) {
            syslog(LOG_ERR, "accept failed");
            return -1;
        }

    
}