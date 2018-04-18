#include "cimple_auxiliary_functions.h"


/**
 * @brief Timer simulating one time step in discrete control
 * @param arg
 * @return
 */
void * timer(void *arg)
{
    struct timeval sec;
    double* total_time_p = (double*)arg;
    double total_time = *total_time_p;
    int seconds = (int)floor(total_time);
    int u_seconds;
    u_seconds = (int)floor(total_time * pow(10,6) - seconds * pow(10,6));
    sec.tv_sec = seconds;
    sec.tv_usec = u_seconds;
    printf("\nTime runs: %d.%d\n", (int)sec.tv_sec, (int)sec.tv_usec);
    select(0,NULL,NULL,NULL,&sec);
    pthread_exit(0);
}


/**
 * @brief Generate gaussian distributed random variable
 * @param mu
 * @param sigma
 * @return
 */
double randn (double mu,
              double sigma)
{
    double U1, U2, W, mult;
    static double X1, X2;
    static int call = 0;

    if (call == 1)
    {
        call = !call;
        return (mu + sigma * (double) X2);
    }

    do
    {
        U1 = -1 + ((double) rand () / RAND_MAX) * 2;
        U2 = -1 + ((double) rand () / RAND_MAX) * 2;
        W = pow (U1, 2) + pow (U2, 2);
    }
    while (W >= 1 || W == 0);

    mult = sqrt ((-2 * log (W)) / W);
    X1 = U1 * mult;
    X2 = U2 * mult;

    call = !call;

    return (mu + sigma * (double) X1);
}

/**
 * Returns 1 (true) if the mutex is unlocked, which is the
 * thread's signal to terminate.
 */
int needQuit(pthread_mutex_t *mtx)
{
    switch(pthread_mutex_trylock(mtx)){
        case 0: /* if we got the lock, unlock and return 1 (true) */
            pthread_mutex_unlock(mtx);
            return 1;
        case EBUSY: /* return 0 (false) if the mutex was locked */
            return 0;
        default:
            return 1;
    }
}