#ifndef SERVER_H
#define SERVER_H


#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include "thread.h"

typedef int socketfd_t;

socketfd_t accepfd;

#define LISTENQ 1024


#define writeout(output, ...)               \
    do {                                    \
        int i;                              \
        char* tmp = (output);               \
        i=asprintf(&(output), __VA_ARGS__); \
        if (i < 0)                          \
        perror("asprintf failure:\n");      \
        free(tmp);                          \
        }while (0)

/**
 * @brief Open a socket to to listen
 * 
 * To get address independent socket, we need the socket len to be
 * returned that we can use in the accept.
 */

socketfd_t open_conn(char const* host, char const* port, socklen_t* addsize);

extern void serve(void* thrd_s, int fd, http_struct* http_s);

void cleanup(void);

char* srv_date(void);

struct {
  char* ext;
  char* filetype;
} extensions [] = {
  {"gif", "image/gif" },
  {"jpg", "image/jpg" },
  {"jpeg","image/jpeg"},
  {"png", "image/png" },
  {"ico", "image/ico" },
  {"zip", "image/zip" },
  {"gz",  "image/gz"  },
  {"tar", "image/tar" },
  {"htm", "text/html" },
  {"html","text/html" },
  {"pdf", "application/pdf" },
  {0,0} };

#endif //SERVER_H
