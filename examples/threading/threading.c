#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // Done: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    
    int rc;
    int msec;

    thread_func_args->thread_complete_success = false;
    msec = usleep(1000 * thread_func_args->wait_to_obtain_ms);

    //Wait until we are done sleeping
    if (msec != 0) {
        DEBUG_LOG("Waiting...");
        return thread_param;
    }

    //Try to obtain mutex
    rc = pthread_mutex_lock(thread_func_args->mutex);
    if (rc != 0) {
        DEBUG_LOG("Could not obtain mutex...");
        return thread_param;
    }

    //Wait until we are done with the mutex
    msec = usleep(1000 * thread_func_args->wait_to_release_ms);
    if (msec != 0) {
        DEBUG_LOG("Waiting...");
        return thread_param;
    }

    //Try to release mutex
    rc = pthread_mutex_unlock(thread_func_args->mutex);
    if (rc != 0) {
        DEBUG_LOG("Could not release mutex...");
        return thread_param;
    }

    thread_func_args->thread_complete_success = true;

    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * Done: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */

    // Allocate memory for thread_data and make sure it worked
    struct thread_data *thread_func_args = malloc(sizeof *thread_func_args);
    if (thread_func_args == NULL) {
        ERROR_LOG("ERROR: Couldn't allocate memory for thread");
        return false;
    }

    // Setup arguments
    thread_func_args->mutex = mutex;
    thread_func_args->wait_to_obtain_ms = wait_to_obtain_ms;
    thread_func_args->wait_to_release_ms = wait_to_release_ms;
    thread_func_args->thread_complete_success = false;

    // Create the thread and error if it fails.
    if (pthread_create(thread, NULL, threadfunc, thread_func_args)) {
        ERROR_LOG("ERROR: Couldn't create thread");
        return false;
    }

    return true;
}

