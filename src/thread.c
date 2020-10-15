#include "thread.h"


pthread_attr_t thrd_attr = {{0}};
pthread_key_t thrd_key1 = 0;
pthread_t thrd_id[THREAD_NUM] = {0};
thrd_data* _dta = NULL;

pthread_once_t thrd_once = PTHREAD_ONCE_INIT;

pthread_mutex_t thrd_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t serv_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;


thrd_data* init_thrd_data(void (*s)(void* ,int ,http_struct*))
{
    thrd_data* _dt = NULL;
    _dt = malloc(sizeof(struct thrd_data));
    if (_dt == NULL) {
        perror("INIT MALLOC ERROR\n");
        exit(1);
    }

    _dt->create_thread = &create_thread;
    _dt->exec_thread = exec_thread;
    _dt->thrd_key_destructor = &thrd_key_destructor;
    _dt->thrd_once_func = &thrd_once_func;
    _dt->thrd_printf = &thrd_printf;
    _dt->srv_func = s;
    _dt->destructor_thrd_data = &destructor_thrd_data;
    _dt->cleanup = &cleanup;
    _dt->set_l_fd = &set_l_fd;
    _dt->get_l_fd = &get_l_fd;

    _dt->per_thrd_dt = NULL;
    _dt->l_fd = 0;

    return _dt;
}


static void destructor_thrd_data (void)
{
    if (_dta != NULL ) {
        free(_dta);
    }
}


void init_per_thrd_data(per_thrd_data* _pdtf,void (*s)(void*, int,
            http_struct* ))
{
    void* h = NULL;
    _pdtf->srv_func = s;
    _pdtf->set_acpt_fd = &set_acpt_fd;
    _pdtf->get_acpt_fd = &get_acpt_fd;
    _pdtf->set_c_sklen = &set_c_sklen;
    _pdtf->get_c_sklen = &get_c_sklen;
    _pdtf->set_c_addr = &set_c_addr;
    _pdtf->get_c_addr = &get_c_addr;
    _pdtf->set_http_s = &set_http_s;
    _pdtf->get_http_s = &get_http_s;

    _pdtf->acpt_fd = 0;
    _pdtf->c_sklen = 0;
    _pdtf->http_s = NULL;

    h = malloc(sizeof(http_struct));
    if (h == NULL) {
        _dta->thrd_printf("MALLOC ERROR: allocating for https\n");
        pthread_exit(NULL);
    }
    _pdtf->set_http_s( _pdtf,(http_struct*)h);
}


static void create_thread(int tid, thrd_data* thrd_dt)
{
    int n;
    if ( (n = pthread_create(&thrd_id[tid], NULL, exec_thread,
                    (void *)thrd_dt )) == 0) {
        return;
    }
}

static void thrd_once_func(void)
{
    pthread_key_create(&thrd_key1, _dta->thrd_key_destructor);
}

static void thrd_key_destructor(void* ptr)
{
    per_thrd_data* td = (per_thrd_data*) ptr;
    if (td->http_s != NULL) {
        free(td->http_s);
        td->http_s = NULL;
    }
    if (td != NULL) {
        free(td);
        td = NULL;
    }
}



static void* exec_thread(void* args)
{
    thrd_data* dta = (thrd_data*)args;
    per_thrd_data* _pdt = NULL;//dta->get_per_thrd_dt();

    pthread_detach(pthread_self());
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    pthread_once(&thrd_once, dta->thrd_once_func);
    pthread_mutex_lock(&thrd_mutex);
    if ((_pdt = pthread_getspecific(thrd_key1)) == NULL) {
        _pdt = malloc( sizeof(per_thrd_data));
        if (_pdt == NULL) {
            pthread_mutex_unlock(&serv_mutex);
            dta->thrd_printf("tdata: malloc failure!\n");
            pthread_exit(NULL);
        }
        pthread_setspecific(thrd_key1, _pdt);
    }
    init_per_thrd_data(_pdt, dta->srv_func);
    pthread_mutex_unlock(&thrd_mutex);

    while (true) {
        _pdt->set_acpt_fd(_pdt,accept(dta->get_l_fd(), _pdt->get_c_addr(_pdt),
                    _pdt->get_c_sklen(_pdt)));

        _pdt->srv_func(_pdt, _pdt->get_acpt_fd(_pdt),_pdt->get_http_s(_pdt));

        shutdown(_pdt->get_acpt_fd(_pdt), SHUT_RDWR);
        sleep(1);
        close(_pdt->get_acpt_fd(_pdt));
    }
}


static void cleanup(void)
{
    size_t i;
    for (i = 0; i < THREAD_NUM;++i) {
        if (thrd_id[i] != 0) {
            if (pthread_cancel(thrd_id[i]) ) {
                _dta->thrd_printf("pthread cancel error\n");
            }
        }
    }
}

static void set_acpt_fd(per_thrd_data* _pdt, int val)
{
    _pdt->acpt_fd = val;
}


static int get_acpt_fd(per_thrd_data* _pdt)
{
    return _pdt->acpt_fd;
}


static void set_c_sklen(per_thrd_data* _pdt, socklen_t val)
{
    _pdt->c_sklen = val;
}


static socklen_t* get_c_sklen(per_thrd_data* _pdt)
{
    return &_pdt->c_sklen;
}


static void set_c_addr(per_thrd_data* _pdt, struct sockaddr val)
{
    _pdt->c_addr = val;
}


static struct sockaddr* get_c_addr(per_thrd_data* _pdt)
{
    return &_pdt->c_addr;
}


static void set_http_s(per_thrd_data* _pdt, http_struct* val)
{
    _pdt->http_s = val;
}


static http_struct* get_http_s(per_thrd_data* _pdt)
{
    return _pdt->http_s;
}


static void set_l_fd(int val)
{
    _dta->l_fd = val;
}

static int get_l_fd(void)
{
    return _dta->l_fd;
}

static void set_per_thrd_dt(per_thrd_data* val)
{
    _dta->per_thrd_dt = val;
}

static per_thrd_data* get_per_thrd_dt(void)
{
    return _dta->per_thrd_dt;
}


static void thrd_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    FILE* filelog = NULL;
    if ((filelog = fopen(LOGF , "a" )) == NULL) {
        perror("ERROR: Not able to open file\n");
        exit(1);
    }
    pthread_mutex_lock(&print_mutex);
    vfprintf(filelog,fmt,ap);
    pthread_mutex_unlock(&print_mutex);

    va_end(ap);
    fclose(filelog);
}


