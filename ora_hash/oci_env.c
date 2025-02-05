/*******************************************************************************
 * oci_env.c
 * Thread-safe, on-demand initialization of a global OCI environment:
 *   - A single global OCIEnv* (created once, lazily)
 *   - One OCIError* per thread, stored in thread-local storage (TLS)
 ******************************************************************************/
#include <stdio.h>
#include <pthread.h>
#include <oci.h>
#include "oci_env.h"
#include "oci_utils.h"


// Global OCI environment pointer
static OCIEnv *g_env_ptr = NULL;

// Flags to track if we've created environment and pthread key
static int g_env_initialized = 0;
static int g_key_created = 0;

// Pthread key for storing each thread's OCIError
static pthread_key_t g_error_key;

// Mutex for double-checked locking
static pthread_mutex_t g_init_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * thread_error_destructor:
 * frees the OCIError handle for a thread when that thread exits.
 */
static void thread_error_destructor(void *error_handle) {
    if (error_handle != NULL) {
        OCIHandleFree(error_handle, OCI_HTYPE_ERROR);
    }
}

/**
 * initialize_oci_env:
 *  - Double-checked locking to ensure OCI environment is initialized once
 *  - Creates a pthread key to store each thread's OCIError handle
 *  - Returns OCI_SUCCESS(0) on success, OCI_ERROR(-1) on failure
 */
static int initialize_oci_env(void) {
    if (g_env_initialized) {
        // Already done
        return OCI_SUCCESS;
    }

    // Lock for double-checked initialization
    pthread_mutex_lock(&g_init_mutex);

    if (!g_env_initialized) {
        // Still not initialized, do it now:
        sword status = OCIInitialize(OCI_THREADED | OCI_OBJECT, NULL, NULL, NULL, NULL);
        if (status != OCI_SUCCESS) {
            print_oci_error(NULL, status, "OCIInitialize() failed");
            pthread_mutex_unlock(&g_init_mutex);
            return OCI_ERROR;
        }
        // Init
        status = OCIEnvInit(&g_env_ptr, OCI_THREADED, 0, NULL);
        if (status != OCI_SUCCESS) {
            print_oci_error(NULL, status, "OCIEnvInit() failed");
            pthread_mutex_unlock(&g_init_mutex);
            return OCI_ERROR;
        }
        // Create a pthread key to store each thread's error handle
        status = pthread_key_create(&g_error_key, thread_error_destructor);
        if (status != 0) {
            print_oci_error(NULL, status, "pthread_key_create() failed");
            pthread_mutex_unlock(&g_init_mutex);
            return OCI_ERROR;
        }
        g_key_created = 1;
        g_env_initialized = 1;
    }

    pthread_mutex_unlock(&g_init_mutex);
    return OCI_SUCCESS;
}

/*******************************************************************************
 * get_oci_env
 *  - Ensures environment is initialized (on demand), then returns g_env_ptr
 *  - Returns OCIEnv or NULL if init fails
 ******************************************************************************/
OCIEnv *get_oci_env(void) {
    if (initialize_oci_env() == OCI_SUCCESS) {
        return g_env_ptr;
    }
    return NULL;
}

/*******************************************************************************
 * get_oci_error_handler
 *  - Ensures environment is inited, retrieves (or creates) an OCIError for
 *    the calling thread, stored in TLS.
 *  - Returns OCIError or NULL on error
 ******************************************************************************/
OCIError *get_oci_error_handler(void) {
    sword status = initialize_oci_env();
    if (status != OCI_SUCCESS) {
        print_oci_error(NULL, status, "Environment init failed in get_oci_error_handler.");
        return NULL;
    }

    if (!g_key_created) {
        print_oci_error(NULL, -1, "Error key was not created.");
        return NULL;
    }

    // Check if we already have an error handle in this thread
    OCIError *err_p = (OCIError *) pthread_getspecific(g_error_key);
    if (err_p != NULL) {
        return err_p;
    }

    // Not allocated => create a new handle
    status = OCIHandleAlloc(g_env_ptr, (void **) &err_p, OCI_HTYPE_ERROR, 0, NULL);
    if (status != OCI_SUCCESS) {
        print_oci_error(NULL, status, "OCIHandleAlloc(OCI_HTYPE_ERROR) failed");
        return NULL;
    }

    // Store it in thread-local
    status = pthread_setspecific(g_error_key, err_p);
    if (status != OCI_SUCCESS) {
        print_oci_error(NULL, status, "pthread_setspecific() failed");
        OCIHandleFree(err_p, OCI_HTYPE_ERROR);
        return NULL;
    }

    return err_p;
}
