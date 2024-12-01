/* utils.c */
/**
 * This file contains various helper functions used throughout the HTTP server
 * application. 
**/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include "logging.h"

typedef struct http_response
{
    // HTTP response fields
    char *status;

    char *content_type;
    char *body;

} http_response;

char *prepare_response(char *status, char *content_type, char *body)
{
    char *response = (char *)malloc(strlen(status) + strlen(content_type) + strlen(body) + 256);
    sprintf(response, "HTTP/1.1 %s\r\nContent-Type: %s\r\n\r\n%s\r\n\r\n", status, content_type, body);
    return response;
}

typedef struct httpd_request
{
    char *method;
    char *path;
    char *protocol;
    char *headers;
} httpd_request;

int parse_request(int newsockfd, httpd_request *request)
{
    char buffer[256];
    bzero(buffer, 256);
    int n = read(newsockfd, buffer, 255);
    if (n < 0)
    {
        log_error("Error reading from socket", NULL);
        return -1;
    }
    // Parse the request
    request->method = strtok(buffer, " ");
    request->path = strtok(NULL, " ");
    request->protocol = strtok(NULL, "\r\n");
    if (strcmp(request->path, "/") == 0 || request->path == NULL)
    {
        request->path = "/index.html";
    }
    // Parse headers
    char *line = strtok(NULL, "\r\n");
    while (line != NULL)
    {
        request->headers = line;
        line = strtok(NULL, "\r\n");
    }
    // Log the parsed request
    char log_message[512];
    snprintf(log_message, sizeof(log_message), "Received Request: method -> %s path -> %s protocol -> %s \nHeaders:\n%s", request->method, request->path, request->protocol, request->headers);
    log_info(log_message);
    return 0;
}

int read_binary_file(const char *path, char **buffer, size_t *size) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        log_error("Error opening file", NULL);
        return -1;
    }

    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);

    *buffer = (char *)malloc(*size);
    if (*buffer == NULL) {
        log_error("Memory allocation failed", NULL);
        fclose(file);
        return -1;
    }

    fread(*buffer, 1, *size, file);
    fclose(file);
    return 0;
}

int read_file(char *path, char *buffer)
{
    log_info(path);
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        log_error("Error opening file", NULL);
        return -1;
    }
    buffer[0] = '\0';
    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        strcat(buffer, line);
    }
    fclose(file);
    return 0;
}

void process_request(int newsockfd, char *path)
{
    struct httpd_request *request = (struct httpd_request *)malloc(sizeof(struct httpd_request));
    if (parse_request(newsockfd, request) != 0)
    {
        log_error("Error parsing request", NULL);
        free(request);
        return;
    }
    if (request->path != NULL)
    { // Return a simple HTTP response
        char *body = (char *)malloc(1024);

        // Get path to path + index.html
        char *filepath = (char *)malloc(256);
        sprintf(filepath, "%s%s", path, request->path);
        if (read_file(filepath, body) != 0)
        {
            // Handle file read error
            free(filepath);
            free(body);
            free(request);

            return;
        }

        char *response = prepare_response("200 OK", "text/html", body);
        // log_info(response);
        write(newsockfd, response, strlen(response));

        free(body);
        free(response);
        free(request);
        return;
    }

    // Return a 404 response
    log_info("404 Not Found");
    char *response = prepare_response("404 Not Found", "text/html", "<h1>404 Not Found</h1>");
    write(newsockfd, response, strlen(response));
    free(response);
    free(request);
    return;
}
