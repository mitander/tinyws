#include "include/sha1.h"
#include <stddef.h>

#define SHA1_CIRCULAR_SHIFT(bits, word) (((word) << (bits)) | ((word) >> (32 - (bits))))

const uint32_t initial_hash[SHA1_HASH_SIZE / 4] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476,
                                                   0xC3D2E1F0};

void msg_transform(struct sha_ctx *);
void msg_process_block(struct sha_ctx *);

int sha1_reset(struct sha_ctx *ctx)
{
    int i;

    if(!ctx)
    {
        return SHA1_ERR_NULL;
    }

    for(i = 0; i < SHA1_HASH_SIZE / 4; i++)
    {
        ctx->hash[i] = initial_hash[i];
    }

    ctx->low_len = 0;
    ctx->high_len = 0;
    ctx->msg_block_idx = 0;
    ctx->length = 0;

    return SHA1_ERR_OK;
}

int sha1_result(struct sha_ctx *ctx, uint8_t msg_digest[SHA1_HASH_SIZE])
{
    int i;

    if(!ctx || !msg_digest)
    {
        return SHA1_ERR_NULL;
    }

    if(!ctx->length)
    {
        msg_transform(ctx);

        for(i = 0; i < 64; ++i)
        {
            ctx->msg_block[i] = 0;
        }

        ctx->low_len = 0;
        ctx->high_len = 0;
        ctx->length = 1;
    }

    for(i = 0; i < SHA1_HASH_SIZE; ++i)
    {
        msg_digest[i] = ctx->hash[i >> 2] >> 8 * (3 - (i & 0x03));
    }

    return SHA1_ERR_OK;
}

int sha1_input(struct sha_ctx *ctx, const uint8_t *msg_arr, unsigned len)
{
    if(!len)
    {
        return SHA1_ERR_OK;
    }

    if(!ctx || !msg_arr)
    {
        return SHA1_ERR_NULL;
    }

    if(ctx->length)
    {
        return SHA1_ERR_STATE;
    }

    for(int i = 0; i < len; i++)
    {
        ctx->msg_block[ctx->msg_block_idx++] = (*msg_arr & 0xFF);
        ctx->low_len += 8;

        if(!ctx->low_len)
        {
            ctx->high_len++;
            if(!ctx->high_len)
            {
                return SHA1_ERR_INPUT_TOO_LONG;
            }
        }

        if(ctx->msg_block_idx == 64)
        {
            msg_process_block(ctx);
        }

        msg_arr++;
    }
    return SHA1_ERR_OK;
}

void msg_process_block(struct sha_ctx *ctx)
{
    int i, sum;
    uint32_t W[80];
    uint32_t A, B, C, D, E, tmp;

    A = ctx->hash[0];
    B = ctx->hash[1];
    C = ctx->hash[2];
    D = ctx->hash[3];
    E = ctx->hash[4];

    for(i = 0; i < 80; i++)
    {
        if(i < 16)
        {
            W[i] = ctx->msg_block[(ptrdiff_t) i * 4] << 24;
            W[i] |= ctx->msg_block[i * 4 + 1] << 16;
            W[i] |= ctx->msg_block[i * 4 + 2] << 8;
            W[i] |= ctx->msg_block[i * 4 + 3];
        }
        else
        {
            W[i] = SHA1_CIRCULAR_SHIFT(1, W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]);
        }
    }

    for(i = 0; i < 80; i++)
    {
        if(i < 20)
        {
            sum = ((B & C) | ((~B) & D)) + E + W[i] + 0x5A827999;
        }
        else if(i < 40)
        {
            sum = (B ^ C ^ D) + E + W[i] + 0x6ED9EBA1;
        }
        else if(i < 60)
        {
            sum = ((B & C) | (B & D) | (C & D)) + E + W[i] + 0x8F1BBCDC;
        }
        else
        {
            sum = (B ^ C ^ D) + E + W[i] + 0xCA62C1D6;
        }

        tmp = SHA1_CIRCULAR_SHIFT(5, A) + sum;
        E = D;
        D = C;
        C = SHA1_CIRCULAR_SHIFT(30, B);
        B = A;
        A = tmp;
    }

    ctx->hash[0] += A;
    ctx->hash[1] += B;
    ctx->hash[2] += C;
    ctx->hash[3] += D;
    ctx->hash[4] += E;

    ctx->msg_block_idx = 0;
}

void msg_transform(struct sha_ctx *ctx)
{
    if(ctx->msg_block_idx > 55)
    {
        ctx->msg_block[ctx->msg_block_idx++] = 0x80;
        while(ctx->msg_block_idx < 64)
        {
            ctx->msg_block[ctx->msg_block_idx++] = 0;
        }

        msg_process_block(ctx);
        while(ctx->msg_block_idx < 56)
        {
            ctx->msg_block[ctx->msg_block_idx++] = 0;
        }
    }
    else
    {
        ctx->msg_block[ctx->msg_block_idx++] = 0x80;
        while(ctx->msg_block_idx < 56)
        {
            ctx->msg_block[ctx->msg_block_idx++] = 0;
        }
    }

    ctx->msg_block[56] = ctx->high_len >> 24;
    ctx->msg_block[57] = ctx->high_len >> 16;
    ctx->msg_block[58] = ctx->high_len >> 8;
    ctx->msg_block[59] = ctx->high_len;
    ctx->msg_block[60] = ctx->low_len >> 24;
    ctx->msg_block[61] = ctx->low_len >> 16;
    ctx->msg_block[62] = ctx->low_len >> 8;
    ctx->msg_block[63] = ctx->low_len;

    msg_process_block(ctx);
}
