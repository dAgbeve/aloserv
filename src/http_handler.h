#ifndef HTTP_HANDLER_H
#define HTTP_HANDLER_H


#include <stdio.h>
#include <time.h>
#include <string.h>

#define BUFFSIZE 1024

#define MKSTR(s)    #s

#define VERSION     MKSTR(0.0)
#define SERVER      MKSTR(ALOWEB/)VERSION
#define H_404       MKSTR(HTTP/1.1 404 Not Found\n)
#define H_200       MKSTR(HTTP/1.1 200 OK\n)

#define B_404       MKSTR(<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">\
        <html><head><title>404 Not Found</title></head><body>\
        <h1>Not Found</h1><p>The requested URL /t.html was not found on this \
        server.</p></body></html>)

#define CON_CLOSE   MKSTR(Connection: Closed)
#define C_TYPE      MKSTR(Content-Type: text/html; charset=iso-8859-1)
#define C_IMAGE     MKSTR(Content-Type: image/apng)
#define GET "GET "


typedef struct http_struct {
    char  recv_buff[BUFFSIZE];
    char* send_buff;
    char* header;
    char* body;
}http_struct;

#endif //HTTP_HANDLER_H
