#define _GNU_SOURCE

#include "server.h"
#include <asm-generic/socket.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>


int main(int argc, char* argv[])
{
    size_t i;
    int ret;
    socklen_t srvsize;
    sigset_t sigset;
    int sig;
    FILE* logfile = NULL;
    int* sigptr = &sig;
    char const* serv = "80";
    char const* host = NULL;
    struct stat st = {0};
    _dta = init_thrd_data(serve);

    if (stat(LOGDIR, &st) == 0 && S_ISDIR(st.st_mode)) {
        logfile = fopen(LOGF, "a");
        if (logfile == NULL) {
            printf("Logging directory exist but could not create the log"
                    " file\n");
            exit(1);
        }
        fprintf(logfile, "this is much better %ld\n", pthread_self() );
        fclose(logfile);
    } else {
        ret = mkdir(LOGDIR, 0777);
        if (!ret)
            printf("directory created\n");
        else {
            printf("Unable to create directory ... Exiting\n");
            exit(1);
        }

    }

    if (stat(WKDIR, &st) == 0 && S_ISDIR(st.st_mode)) {
        _dta->thrd_printf("Logging directory exist\n");

    } else {
        ret = mkdir(WKDIR, 0777);
        if (!ret)
            _dta->thrd_printf("directory created\n");
        else {
        _dta->thrd_printf("Unable to create directory ... Exiting\n");
        exit(1);
        }
    }

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    sigaddset(&sigset, SIGTERM);
    sigaddset(&sigset, SIGQUIT);
    sigaddset(&sigset, SIGTSTP);
    pthread_sigmask(SIG_BLOCK,&sigset,NULL);

    _dta->set_l_fd(open_conn(host, serv,&srvsize));

    if (daemon(0, 0) < 0)           //to use this we need to have a logfile
        _dta->thrd_printf("Could not daemonize\n"); 
    for (i = 0; i < THREAD_NUM; i++)
        _dta->create_thread(i, _dta);


    pthread_sigmask(SIG_SETMASK, &sigset, NULL);
    ret = sigwait(&sigset, sigptr);
    if ( ret != 0)
        perror("sigwait failed\n");
    if (*sigptr == SIGINT || *sigptr == SIGTERM || *sigptr == SIGQUIT ||
            *sigptr == SIGTSTP) {
        _dta->cleanup();
        sleep(1);
        _dta->destructor_thrd_data ();
        pthread_exit(NULL);
    }
}



void serve(void* thrd_s, int fd, http_struct* http_s)
{
    long ret = 0, len = 0, i, b_len,j;
    char* ftype;
    char* d;
    per_thrd_data* thrd_dt = (per_thrd_data*) thrd_s;
    FILE* file;

    http_s->send_buff = NULL;

    ret = chdir(WKDIR);
    if (ret != 0){
        _dta->thrd_printf("Error: Changing directory\n");
        pthread_exit(NULL);
    }

    ret = read(fd, http_s->recv_buff, BUFFSIZE);
    if (ret == 0 || ret == -1) {
        _dta->thrd_printf("read from socket failure\n");
        goto out;
    }

    if (ret > 0 && ret < BUFFSIZE)
        http_s->recv_buff[ret] = 0;
    else
        http_s->recv_buff[0] = 0;


    for (i = 0; i < ret; i++)
        if (http_s->recv_buff[i] == '\r' || http_s->recv_buff[i] == '\n')
            http_s->recv_buff[i] = '*';

     _dta->thrd_printf("%s\n",http_s->recv_buff);

     if (strncmp(http_s->recv_buff, GET, 4))
         _dta->thrd_printf("only get operations are allowed\n");

     for (i = 4; i < BUFFSIZE; ++i) {
         if (http_s->recv_buff[i] == ' ') {
             http_s->recv_buff[i] = '\0';
             break;
         }
     }

     for (j = 0; j < i -1; j++)
         if (http_s->recv_buff[j] == '.' && http_s->recv_buff[j+1] == '.')
             _dta->thrd_printf("Access to directory above is not allowed\n");

     if (!strncmp(http_s->recv_buff,"GET /\0",6))
         strncpy(http_s->recv_buff, "GET /index.html",16 );

     b_len = strlen(http_s->recv_buff);
     ftype = NULL;
     for (i = 0; extensions[i].ext != 0; ++i) {
         len = strlen(extensions[i].ext);
         if (!strncmp(&http_s->recv_buff[b_len - len], extensions[i].ext, len)) {
             ftype = extensions[i].filetype;
             _dta->thrd_printf("ftype : %s\n", ftype);
             break;
         }
     }

     if (ftype == NULL) {
         _dta->thrd_printf("not supported file type\n");
         goto fileError;

     }

     if (!strncmp(ftype, "text", 4))
         file = fopen(&http_s->recv_buff[5], "r");
     else
         file = fopen(&http_s->recv_buff[5], "rb");

     if (file  == NULL) {
fileError:
         d = srv_date();
         len = strlen(B_404);
         writeout(http_s->send_buff,"%sDate: %sServer: %s\nContent-Length: "
                 "%ld\n%s\n%s\n\n%s\n", H_404 ,d, SERVER, len,CON_CLOSE,C_TYPE
                 ,B_404);
         _dta->thrd_printf("%s\nsock file : %d\n", http_s->send_buff,fd);
         _dta->thrd_printf("open called len = %ld\n ",len);
         len = strlen(http_s->send_buff);
         _dta->thrd_printf("open called len 2 = %ld\n",len);
         ret = write(thrd_dt->get_acpt_fd(thrd_dt), http_s->send_buff, len);
         if (ret < 0) {
             _dta->thrd_printf("write to socket error\n");
             goto out;
         }
         goto out;
     }

     fseek(file, 0, SEEK_END);
     b_len = ftell(file);
     rewind(file); 
     
     d = srv_date();
     writeout(http_s->send_buff,"%sDate: %sServer: %s\nContent-Length: %ld\n"
             "%s\nContent-Type: %s\n\n",H_200, d, SERVER, b_len,CON_CLOSE,
             ftype);
     len = strlen(http_s->send_buff);

     http_s->send_buff = (char*)realloc(http_s->send_buff,len + b_len);

     ret = fread(&http_s->send_buff[len], 1, b_len, file);
     if (ret != b_len)
         _dta->thrd_printf("fread error\n");

     send(thrd_dt->get_acpt_fd(thrd_dt), http_s->send_buff, len +b_len, 0);
     if (fclose(file) != 0)
         _dta->thrd_printf("error closing file \n");

out:
     if (http_s->send_buff != NULL) {
         free(http_s->send_buff);
         http_s->send_buff = NULL;
     }
}



socketfd_t open_conn(const char *host, const char *port, socklen_t *addsize)
{
    socketfd_t listenfd = 0;
    int n;
    int const on = 1;
    struct addrinfo hints,* r, *r_s;

    memset(&hints, '\0', sizeof( struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (( n = getaddrinfo(host, port, &hints, &r)) != 0 ) {
        fprintf(stderr, "ERROR open_conn: host - %s port - %s"
                " errno - %s\n", host, port, gai_strerror(n));
    }
    r_s = r;

    do {
        listenfd = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
        if (listenfd < 0)
            continue;

        if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
            fprintf(stderr, "setsockopt error\n");
        
        if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) < 0)
            fprintf(stderr, "setsockopt error\n");

        if (bind(listenfd,r->ai_addr, r->ai_addrlen) == 0)
            break;

        close(listenfd);
    }while ((r = r->ai_next) != NULL);

    if (r == NULL)
        fprintf(stderr, "ERROR open_conn: host - %s port - %s\n",
                host, port);

    if ( listen(listenfd, LISTENQ) < 0)
        fprintf(stderr, "listen error\n");

    if (addsize)
        *addsize = r->ai_addrlen;

    freeaddrinfo(r_s);

    return listenfd;
}

char* srv_date (void)
{
    time_t t = time(NULL);
    struct tm* s_t = gmtime(&t);
    return asctime(s_t);
}
