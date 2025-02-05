#ifndef OCI_UTILS_H
#define OCI_UTILS_H

#include <oci.h>

#ifdef __cplusplus
extern "C" {
#endif

void print_oci_error(OCIError *err, sword status, const char *message);

#ifdef __cplusplus
}
#endif

#endif // OCI_UTILS_H
