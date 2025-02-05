#ifndef OCI_ENV_H
#define OCI_ENV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <oci.h>

/**
 * get_oci_env
 *
 * Returns the global OCIEnv* pointer.
 * The OCI environment is initialized on-demand (thread-safe).
 *
 * @return OCIEnv* on success, or NULL on error
 */
OCIEnv *get_oci_env(void);

/**
 * get_thread_error_handle
 *
 * Returns the calling thread's OCIError* handle, creating it if needed.
 * Each thread gets its own error handle (stored in thread-local storage).
 *
 * @return OCIError* on success, or NULL on error
 */
OCIError *get_oci_error_handler(void);

#ifdef __cplusplus
}
#endif

#endif /* OCI_ENV_H */
