#include "include/sha1.h"

#define SHA1_CIRCULAR_SHIFT(bits, word) (((word) << (bits)) | ((word) >> (32 - (bits))))

void pad_msg(struct sha_ctx *);
void process_msg_block(struct sha_ctx *);

int sha1_reset(struct sha_ctx *ctx)
{
    if(!ctx)
    {
        return SHA_ERR_NULL;
    }

    ctx->low_len = 0;
    ctx->high_len = 0;
    ctx->msg_block_idx = 0;

    ctx->hash[0] = 0x67452301;
    ctx->hash[1] = 0xEFCDAB89;
    ctx->hash[2] = 0x98BADCFE;
    ctx->hash[3] = 0x10325476;
    ctx->hash[4] = 0xC3D2E1F0;

    ctx->count = 0;
    ctx->corrupted = 0;

    return SHA_ERR_OK;
}

int sha1_result(struct sha_ctx *ctx, uint8_t Message_Digest[SHA1_HASH_SIZE])
{
    int i;

    if(!ctx || !Message_Digest)
    {
        return SHA_ERR_NULL;
    }

    if(ctx->corrupted)
    {
        return ctx->corrupted;
    }

    if(!ctx->count)
    {
        pad_msg(ctx);
        for(i = 0; i < 64; ++i)
        {
            ctx->msg_block[i] = 0;
        }
        ctx->low_len = 0;
        ctx->high_len = 0;
        ctx->count = 1;
    }

    for(i = 0; i < SHA1_HASH_SIZE; ++i)
    {
        Message_Digest[i] = ctx->hash[i >> 2] >> 8 * (3 - (i & 0x03));
    }

    return SHA_ERR_OK;
}

int sha1_input(struct sha_ctx *ctx, const uint8_t *message_array, unsigned length)
{
    if(!length)
    {
        return SHA_ERR_OK;
    }

    if(!ctx || !message_array)
    {
        return SHA_ERR_NULL;
    }

    if(ctx->count)
    {
        ctx->corrupted = SHA_ERR_STATE;

        return SHA_ERR_STATE;
    }

    if(ctx->corrupted)
    {
        return ctx->corrupted;
    }
    while(length-- && !ctx->corrupted)
    {
        ctx->msg_block[ctx->msg_block_idx++] = (*message_array & 0xFF);

        ctx->low_len += 8;
        if(ctx->low_len == 0)
        {
            ctx->high_len++;
            if(ctx->high_len == 0)
            {
                // msg too long
                ctx->corrupted = 1;
            }
        }

        if(ctx->msg_block_idx == 64)
        {
            process_msg_block(ctx);
        }

        message_array++;
    }

    return SHA_ERR_OK;
}

void process_msg_block(struct sha_ctx *ctx)
{
    const uint32_t K[] = {0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6};
    int t;
    uint32_t temp;
    uint32_t W[80];
    uint32_t A, B, C, D, E;

    for(t = 0; t < 16; t++)
    {
        W[t] = ctx->msg_block[t * 4] << 24;
        W[t] |= ctx->msg_block[t * 4 + 1] << 16;
        W[t] |= ctx->msg_block[t * 4 + 2] << 8;
        W[t] |= ctx->msg_block[t * 4 + 3];
    }

    for(t = 16; t < 80; t++)
    {
        W[t] = SHA1_CIRCULAR_SHIFT(1, W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16]);
    }

    A = ctx->hash[0];
    B = ctx->hash[1];
    C = ctx->hash[2];
    D = ctx->hash[3];
    E = ctx->hash[4];

    for(t = 0; t < 20; t++)
    {
        temp = SHA1_CIRCULAR_SHIFT(5, A) + ((B & C) | ((~B) & D)) + E + W[t] + K[0];
        E = D;
        D = C;
        C = SHA1_CIRCULAR_SHIFT(30, B);

        B = A;
        A = temp;
    }

    for(t = 20; t < 40; t++)
    {
        temp = SHA1_CIRCULAR_SHIFT(5, A) + (B ^ C ^ D) + E + W[t] + K[1];
        E = D;
        D = C;
        C = SHA1_CIRCULAR_SHIFT(30, B);
        B = A;
        A = temp;
    }

    for(t = 40; t < 60; t++)
    {
        temp = SHA1_CIRCULAR_SHIFT(5, A) + ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
        E = D;
        D = C;
        C = SHA1_CIRCULAR_SHIFT(30, B);
        B = A;
        A = temp;
    }

    for(t = 60; t < 80; t++)
    {
        temp = SHA1_CIRCULAR_SHIFT(5, A) + (B ^ C ^ D) + E + W[t] + K[3];
        E = D;
        D = C;
        C = SHA1_CIRCULAR_SHIFT(30, B);
        B = A;
        A = temp;
    }

    ctx->hash[0] += A;
    ctx->hash[1] += B;
    ctx->hash[2] += C;
    ctx->hash[3] += D;
    ctx->hash[4] += E;

    ctx->msg_block_idx = 0;
}

void pad_msg(struct sha_ctx *ctx)
{
    if(ctx->msg_block_idx > 55)
    {
        ctx->msg_block[ctx->msg_block_idx++] = 0x80;
        while(ctx->msg_block_idx < 64)
        {
            ctx->msg_block[ctx->msg_block_idx++] = 0;
        }

        process_msg_block(ctx);

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

    process_msg_block(ctx);
}
