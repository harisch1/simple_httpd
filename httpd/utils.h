
#ifndef UTILS_H
#define UTILS_H

char *prepare_response(char *status, char *content_type, char *body);

void process_request(int newsockfd, char *path);

#endif