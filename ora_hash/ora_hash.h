#ifndef KGGHASH_H
#define KGGHASH_H

#ifdef __cplusplus
extern "C" {
#endif

int shard_key_in_range(long key1, long low_key, long high_key);

int shard_keys_2_in_range(long key1, long key2, long low_key, long high_key);

int shard_keys_3_in_range(long key1, long key2, long key3, long low_key, long high_key);

#ifdef __cplusplus
}
#endif

#endif /* KGGHASH_H */
