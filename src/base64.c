#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "include/base64.h"

static const unsigned char encode_tbl[65] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static int mod_table[] = {0, 2, 1};

char *base64_encode(const unsigned char *data, size_t len)
{
    int i, j;
    size_t out_len;
    char *encoded_data;

    out_len = 4 * ((len + 2) / 3);

    encoded_data = malloc(out_len);
    if(!encoded_data)
    {
        return NULL;
    }

    for(i = 0, j = 0; i < len;)
    {
        uint32_t octet_a = i < len ? (unsigned char) data[i++] : 0;
        uint32_t octet_b = i < len ? (unsigned char) data[i++] : 0;
        uint32_t octet_c = i < len ? (unsigned char) data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encode_tbl[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encode_tbl[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encode_tbl[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encode_tbl[(triple >> 0 * 6) & 0x3F];
    }

    for(i = 0; i < mod_table[len % 3]; i++)
    {
        encoded_data[out_len - 1 - i] = '=';
    }

    return encoded_data;
}


char *base64_decode(const unsigned char *data, size_t len, size_t *out_len)
{
    int i, j;
    char decode_tbl[64];
    char *decoded_data;

    if(len % 4 != 0)
    {
        return NULL;
    }

    *out_len = len / 4 * 3;

    for(i = 0; i < 64; i++)
    {
        decode_tbl[(unsigned char) encode_tbl[i]] = i;
    }

    if(data[len - 1] == '=')
    {
        (*out_len)--;
    }

    if(data[len - 2] == '=')
    {
        (*out_len)--;
    }

    decoded_data = malloc(*out_len);
    if(!decoded_data)
    {
        return NULL;
    }

    for(i = 0, j = 0; i < len;)
    {
        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decode_tbl[data[i++]];
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decode_tbl[data[i++]];
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decode_tbl[data[i++]];
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decode_tbl[data[i++]];

        uint32_t triple =
          (sextet_a << 3 * 6) + (sextet_b << 2 * 6) + (sextet_c << 1 * 6) + (sextet_d << 0 * 6);

        if(j < *out_len)
        {
            decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        }

        if(j < *out_len)
        {
            decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        }

        if(j < *out_len)
        {
            decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
        }
    }

    return decoded_data;
}
