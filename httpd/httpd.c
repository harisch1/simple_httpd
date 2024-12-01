/* httpd.c */
/* A simple HTTP server daemon implemented in C*/

#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include "logging.h"
#include "utils.h"

#define LISTENADDR "127.0.0.1"

/* Returns 0 on Error. Else returns the int socket file descriptor */
int tcp_sock_init(int port)
{
    int sockfd;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        log_error("Error opening socket", NULL);
        return 0;
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(LISTENADDR);
    serv_addr.sin_port = htons(port);
    char log_message[256];
    snprintf(log_message, sizeof(log_message), "Binding to address: %s:%d", LISTENADDR, port);
    log_info(log_message);
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        close(sockfd);
        char *err = strerror(errno);
        log_error("Error on binding: %s", err);
        exit(0);
    }
    log_info("Listening for incoming connections...");
    listen(sockfd, 5);
    return sockfd;
}

void signal_handler()
{
    // Type of signal received
    char *signal_type;
    signal_type = (errno == SIGINT) ? "SIGINT" : "SIGTERM";
    char log_message[256];
    snprintf(log_message, sizeof(log_message), "Received %s signal", signal_type);
    log_info(log_message);
    log_info("Socket closed");
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd;       // Socket file descriptor: an integer value that uniquely identifies an open socket of the process.
    char *listenport; // Port number to listen on
    char *path;       // Path to serve files from (default: ./app)

    if (argc < 2)
    {
        log_error("Usage: %s <port>\n", NULL);
    }
    else
    {
        listenport = argv[1];
        // build path from commandline argument or use default
        path = (argc > 2) ? argv[2] : "./app";
        char log_message[256];
        snprintf(log_message, sizeof(log_message), "Serving files from: %s", path);
        log_info(log_message);
    }

    sockfd = tcp_sock_init(atoi(listenport));
    if (sockfd <= 0)
    {
        log_error("Error initializing socket", NULL);
    }
    else
    {
        log_info("Server started successfully!");
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
        signal(SIGABRT, signal_handler);
        // at exit, close the socket and unbind
        void close_socket(void)
        {
            close(sockfd);
        };

        if (atexit(close_socket) != 0)
        {
            log_error("Error on close", NULL);
            exit(1);
        };
    }
    while (1)
    {
        int newsockfd;
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
        {
            log_error("Error on accept", NULL);
            exit(0);
        }
        else
        {
            log_info("Connection accepted!");
            // Process the request
            process_request(newsockfd, path);
            // Return a simple HTTP response
            close(newsockfd);
            log_info("Connection Closed");
        }
    }
    return 0;
}
