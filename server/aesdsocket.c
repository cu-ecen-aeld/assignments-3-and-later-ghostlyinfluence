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

#define PORT 9000
#define BUF_LEN 1024

//Globals
int run_flag = 0;
int sockfd = -1;

static void signal_handler(int signo) {
    if (signo == SIGINT || signo == SIGTERM) {
        syslog(LOG_INFO, "Caught signal, exiting");
        // shutdown the socket
        shutdown(sockfd, SHUT_RD);
        run_flag = 1;
    }
}

int main(int argc, char *argv[]) {
    
    // DONE: Logs message to the syslog
    openlog(NULL, 0, LOG_USER);
    syslog(LOG_INFO, "syslog opened for aesdsocket");

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        syslog(LOG_ERR, "socket failed");
        exit(-1);
    }

    // use SO_REUSEADDR to reuse the port
    int s_opt = 1;
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &s_opt, sizeof(s_opt));
    if (ret < 0)
    {
        syslog(LOG_ERR, "setsockopt failed");
        exit(-1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    ret = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret < 0)
    {
        syslog(LOG_ERR, "bind failed");
        exit(-1);
    }

    // // Follow what the video said to do. ALSO, the getaddrinfo man page has a great example
    // struct addrinfo hints;
    // struct addrinfo *servinfo, *rp;
    // int status;
    
    // //Setup the socket 'hints'
    // memset(&hints, 0, sizeof(hints));
    // hints.ai_flags = AI_PASSIVE;
    // hints.ai_family = AF_INET;
    // hints.ai_socktype = SOCK_STREAM;
    // hints.ai_protocol = 0;
    // hints.ai_addrlen = 0;
    // hints.ai_addr = NULL;
    // hints.ai_canonname = NULL;
    // hints.ai_next = NULL;

    // // getaddrinfo step
    // if ((status = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
    //     syslog(LOG_ERR, "getaddrinfo: %s\n", gai_strerror(status));
    //     exit(-1);
    // }

    // // open socket step
    // for (rp = servinfo; rp != NULL; rp = rp->ai_next) {
    //     sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    //     if (sockfd == -1) {
    //         continue;
    //     }

    //     if (bind(sockfd, rp->ai_addr, rp->ai_addrlen) == 0) {
    //         break;
    //     } else {
    //         close(sockfd);
    //         freeaddrinfo(servinfo);
    //         exit(-1);
    //     }
    // }

    // // Verify that it could bind
    // if (rp == NULL) {
    //     syslog(LOG_ERR, "could not bind");
    //     close(sockfd);
    //     freeaddrinfo(servinfo);
    //     exit(-1);
    // }

    // Setup Daemon if requested
    int opt;
    while ((opt = getopt(argc, argv, "d")) != -1) {
        switch (opt) {
        case 'd': 
            if (daemon(0, 0) < 0) {
                syslog(LOG_ERR, "daemon failed");
                exit(-1);
            }
            break;
        default:
            printf("Usage: %s [-d]\n", argv[0]);
            exit(-1);
        }
    }

    // Listen
    int backlog = 128; //Number from manpage
    if ((listen(sockfd, backlog)) == -1) {
        syslog(LOG_ERR, "listen failed");
        close(sockfd);
        exit(-1);
    }

    // assign signal handlers
    struct sigaction signal_action;
    signal_action.sa_handler = signal_handler;
    signal_action.sa_flags = 0;
    sigemptyset(&signal_action.sa_mask);
    sigaction(SIGINT, &signal_action, NULL);
    sigaction(SIGTERM, &signal_action, NULL);

    // Open and create the file if it doesn't already exist
    int fd = open("/var/tmp/aesdsocketdata", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        syslog(LOG_ERR, "open failed");
        exit(-1);
    }

    while(!run_flag) {
        // accept connection
        struct sockaddr_in clientaddr;
        socklen_t clientaddrlen = sizeof(clientaddr);
        int connfd = accept(sockfd, (struct sockaddr *)&clientaddr, &clientaddrlen);
        if (connfd == -1) {
            syslog(LOG_ERR, "accept failed");
            exit(-1);
        }

        // Logs message to the syslog “Accepted connection from xxx” where XXXX is the IP address of the connected client.
        syslog(LOG_INFO, "Accepted connection from %s", inet_ntoa(clientaddr.sin_addr));

        // Handle reading and writing
        char buf[BUF_LEN];
        buf[BUF_LEN - 1] = '\0';
        int eol = 0;
        while(eol == 0) {
            int read_bytes = read(connfd, buf, sizeof(buf));
            if (read_bytes < 0) {
                syslog(LOG_ERR, "read failed");
                exit(-1);
            } else if (read_bytes == 0) {
                //EOF
                break;
            } else {
                // find the new line
                char *new_line_ptr = strchr(buf, '\n');
                if (new_line_ptr != NULL) {
                    read_bytes = (new_line_ptr - buf) + 1;
                    eol = 1;
                }

                int write_bytes = write(fd, buf, read_bytes);
                if (write_bytes < 0) {
                    syslog(LOG_ERR, "write failed");
                    exit(-1);
                }
            }
        }

        lseek(fd, 0, SEEK_SET);

        while(1) {
            int read_bytes = read(fd, buf, sizeof(buf));
            if (read_bytes < 0) {
                syslog(LOG_ERR, "read failed");
                exit(-1);
            } else if (read_bytes == 0) {
                //EOF
                break;
            } else {
                int write_bytes = write(connfd, buf, read_bytes);
                if (write_bytes < 0) {
                    syslog(LOG_ERR, "write failed");
                    exit(-1);
                }
            }
        }

        // syslog that connection closed
        syslog(LOG_INFO, "Closed connection from %s", inet_ntoa(clientaddr.sin_addr));
        close(connfd);
    }

    close(fd);
    close(sockfd);
    unlink("/var/tmp/aesdsocketdata");
    closelog();
    return 0;
}