#ifndef KGG_HASH_H
#define KGG_HASH_H

#include <oratypes.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Hashes the given key and returns a 32-bit value.
 * @param key     Pointer to the data buffer.
 * @param length  Length of the data in bytes.
 * @param init_val Previous hash or arbitrary value (e.g., 0).
 * @return        32-bit hash result.
 */
ub4 kgghash(const void *key, size_t length, ub4 init_val);

#ifdef __cplusplus
}
#endif

#endif // KGG_HASH_H
