#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <unistd.h>
#include "http_handler.h"


#define THREAD_NUM 20


#define LOGDIR  MKSTR(/var/log/)
#define WKDIR   MKSTR(/var/www/)
#define LOGF    MKSTR(/var/log/alserv.log)

//extern pthread_t thrd_id[THREAD_NUM] ;
//extern pthread_attr_t thrd_attr  ;
//extern pthread_key_t thrd_key1;


typedef struct per_thrd_data{

   /*
     * Variables
     * */
    int acpt_fd;   //this is for the accepted file descriptor
    socklen_t c_sklen; //store the size of client address
    struct sockaddr c_addr; // sockaddr struct of a client
    http_struct* http_s; // http struct to handle all the response
    struct thrd_data* thrd_data;
    /*
     * Functions
     * */
    void (*srv_func)(void* , int , http_struct* );
    /*
     * setters and getters
     */
    void (*set_acpt_fd)(struct per_thrd_data* ,int);
    int (*get_acpt_fd)(struct per_thrd_data*);
    void (*set_c_sklen)(struct per_thrd_data* ,socklen_t);
    socklen_t* (*get_c_sklen)(struct per_thrd_data* );
    void (*set_c_addr)(struct per_thrd_data* ,struct sockaddr);
    struct sockaddr* (*get_c_addr)(struct per_thrd_data* );
    void (*set_http_s)(struct per_thrd_data* ,http_struct*);
    http_struct* (*get_http_s)(struct per_thrd_data* );

}per_thrd_data;
//extern per_thrd_data* _pdt;



typedef struct thrd_data{
    /*
     * Functions
     * */
    void (*create_thread)(int , struct thrd_data*);
    void* (*exec_thread)(void* args);
    void (*thrd_key_destructor)(void* ptr);
    void (*thrd_once_func)(void);
    void (*thrd_printf)(const char *fmt, ...);
    void (*cleanup)(void);
    void (*destructor_thrd_data) (void);

    void (*srv_func)(void* , int , http_struct* );
    /*
     * setters and getters
     * */
    void (*set_l_fd)(int);
    int (*get_l_fd)();
    void (*set_per_thrd_dt)(per_thrd_data*);
    per_thrd_data* (*get_per_thrd_dt)();
    /*
     * Variable
     * */
    per_thrd_data* per_thrd_dt;
    int l_fd;
} thrd_data;
extern thrd_data* _dta;


/*
 *
 * Function prototypes
 *
 * */

thrd_data* init_thrd_data(void (*)(void* ,int ,http_struct* ));
static void destructor_thrd_data (void);
static void init_per_thrd_data(per_thrd_data*, 
        void (*)(void* ,int ,http_struct* ));
static void create_thread(int tid, thrd_data*);  //creates THREAD_NUM threads
static void* exec_thread(void* args); // calls to accept and also serve the clients
static void thrd_key_destructor(void* ptr);
static void thrd_once_func(void);
static void cleanup(void);

static void thrd_printf(const char *fmt, ...);

/*
 * Setters and Getters thrd_data
 *
 * */

static void (set_l_fd)(int);
static int (get_l_fd)(void);
static void (set_per_thrd_dt)(per_thrd_data*);
static per_thrd_data* (get_per_thrd_dt)();

/*
 * Setters and Getters for per_thrd_data
 *
 * */
static void set_acpt_fd(per_thrd_data* ,int);
static int get_acpt_fd(per_thrd_data*);
static void set_c_sklen(per_thrd_data* ,socklen_t);
static socklen_t* get_c_sklen(per_thrd_data* );
static void set_c_addr(per_thrd_data* ,struct sockaddr);
static struct sockaddr* get_c_addr(per_thrd_data* );
static void set_http_s(per_thrd_data*,http_struct*);
static http_struct* get_http_s(per_thrd_data* );



#endif 
