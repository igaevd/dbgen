#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "oci_env.h"
#include "oci_utils.h"
#include "kgg_hash.h"

/**
 * convert_long_to_ora_byte:
 *   - Convert a single 'long' to Oracle NUMBER bytes,
 *   - Copy into 'buff' at the given offset
 * Returns how many bytes were written
 */
static size_t convert_long_to_ora_byte(long value,
                                       unsigned char *buff,
                                       size_t buffSize) {
    OCIError *err_handler = get_oci_error_handler();
    if (!err_handler) {
        print_oci_error(NULL, OCI_ERROR, "get_oci_error_handler returned NULL");
        return 0;
    }

    OCINumber oci_number;
    sword status = OCINumberFromInt(err_handler, &value, (uword) sizeof(long),
                                    OCI_NUMBER_SIGNED, &oci_number);
    if (status != OCI_SUCCESS) {
        char error_message[256];
        sprintf(error_message, "OCINumberFromInt failed on value=%ld", value);
        print_oci_error(err_handler, status, error_message);
        return 0;
    }

    ub1 length = oci_number.OCINumberPart[0]; // length in bytes
    if (length > buffSize) {
        print_oci_error(NULL, -1, "Buffer overflow in convert_long_to_ora_byte");
        return 0;
    }
    memcpy(buff, oci_number.OCINumberPart + 1, length);
    return length;
}

/**
 * shard_keys_in_range_impl:
 *  - Hashes the given keys checks if the hash is in the range [low_key..high_key].
 * Returns true if the hash is in the range.
 */
static bool shard_keys_in_range_impl(int n_keys, const long *keys, long low_key, long high_key) {
    unsigned char temp_buf[256];
    size_t total_len = 0;
    // convert keys to Oracle NUMBER bytes
    for (int i = 0; i < n_keys; i++) {
        size_t wrote = convert_long_to_ora_byte(keys[i], temp_buf + total_len, sizeof(temp_buf) - total_len);
        if (wrote == 0) {
            fprintf(stderr, "Conversion error at key index %d\n", i);
            return false;
        }
        total_len += wrote;
    }
    // hash
    ub4 hash_val = kgghash(temp_buf, total_len, 0);
    // check range
    return (long) hash_val >= low_key && (long) hash_val <= high_key;
}

/**
 * For a single key
 */
int shard_key_in_range(long key1, long low_key, long high_key) {
    long keys[1] = {key1};
    return shard_keys_in_range_impl(1, keys, low_key, high_key);
}

/**
 * For two keys
 */
int shard_keys_2_in_range(long key1, long key2, long low_key, long high_key) {
    long keys[2] = {key1, key2};
    return shard_keys_in_range_impl(2, keys, low_key, high_key);
}

/**
 * For three keys
 */
int shard_keys_3_in_range(long key1, long key2, long key3, long low_key, long high_key) {
    long keys[3] = {key1, key2, key3};
    return shard_keys_in_range_impl(3, keys, low_key, high_key);
}


/**
 * main
 * Takes 1 to 3 long values as arguments and prints their hash.
 */
int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 4) {
        fprintf(stderr, "Usage: %s key1 [key2] [key3]\n", argv[0]);
        return 1;
    }

    long keys[3] = {0, 0, 0};
    int n_keys = argc - 1;
    for (int i = 0; i < n_keys; i++) {
        keys[i] = strtol(argv[i + 1], NULL, 10);
    }

    unsigned char temp_buf[256];
    size_t total_len = 0;

    for (int i = 0; i < n_keys; i++) {
        fprintf(stderr, "Value-%d=%ld\n", i+1, keys[i]);
        size_t wrote = convert_long_to_ora_byte(
                keys[i],
                temp_buf + total_len,
                sizeof(temp_buf) - total_len
        );
        if (wrote == 0) {
            fprintf(stderr, "Error converting key at index %d\n", i);
            return 1;
        }
        total_len += wrote;
    }

    ub4 hash_val = kgghash(temp_buf, total_len, 0);
    printf("HASH=%u\n", hash_val);

    return 0;
}
