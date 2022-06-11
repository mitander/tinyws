#ifndef TWS_SHA1_H_
#define TWS_SHA1_H_

#include <stdint.h>

enum
{
    SHA_ERR_OK = 0,
    SHA_ERR_NULL,
    SHA_ERR_INPUT_TOO_LONG,
    SHA_ERR_STATE,
};

#define SHA1_HASH_SIZE 20

struct sha_ctx
{
    uint32_t hash[SHA1_HASH_SIZE / 4];

    uint32_t low_len;
    uint32_t high_len;

    uint16_t msg_block_idx;
    uint8_t msg_block[64];

    int count;
    int corrupted;
};

int sha1_reset(struct sha_ctx *);
int sha1_input(struct sha_ctx *, const uint8_t *, unsigned int);
int sha1_result(struct sha_ctx *, uint8_t msg_digest[SHA1_HASH_SIZE]);

#endif
