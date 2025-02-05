#include <stdio.h>
#include <string.h>
#include <oci.h>
#include "kgg_hash.h"
#include "oci_utils.h"

void print_oci_error(OCIError *err, sword status, const char *message) {
    sb4 errcode;
    if (message != NULL) {
        fprintf(stderr, "ErrorCode=%s Message=%d\n", message, status);
    }
    if (err != NULL && status == OCI_ERROR) {
        text err_buf[512];
        OCIErrorGet((dvoid *) err, (ub4) 1, (text *) NULL, &errcode,
                    err_buf, (ub4) sizeof(err_buf), (ub4) OCI_HTYPE_ERROR);
        fprintf(stderr, "OCIErrorMessage= %s\n", err_buf);
    }
}
