#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "include/base64.h"

static const unsigned char encoding_table[65] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
/* static char *decoding_table = NULL; */
static int mod_table[] = {0, 2, 1};

char *base64_encode(const unsigned char *data, size_t len, size_t *out_len)
{
    *out_len = 4 * ((len + 2) / 3);

    char *encoded_data = malloc(*out_len);
    if(encoded_data == NULL)
        return NULL;

    for(int i = 0, j = 0; i < len;)
    {
        uint32_t octet_a = i < len ? (unsigned char) data[i++] : 0;
        uint32_t octet_b = i < len ? (unsigned char) data[i++] : 0;
        uint32_t octet_c = i < len ? (unsigned char) data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for(int i = 0; i < mod_table[len % 3]; i++)
        encoded_data[*out_len - 1 - i] = '=';

    return encoded_data;
}


unsigned char *base64_decode(const unsigned char *data, size_t input_length, size_t *output_length)
{
    char decoding_table[64];
    for(int i = 0; i < 64; i++)
        decoding_table[(unsigned char) encoding_table[i]] = i;

    printf("it's working????");

    if(input_length % 4 != 0)
        return NULL;

    *output_length = input_length / 4 * 3;
    if(data[input_length - 1] == '=')
        (*output_length)--;
    if(data[input_length - 2] == '=')
        (*output_length)--;

    unsigned char *decoded_data = malloc(*output_length);
    if(decoded_data == NULL)
        return NULL;

    for(int i = 0, j = 0; i < input_length;)
    {
        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

        uint32_t triple =
          (sextet_a << 3 * 6) + (sextet_b << 2 * 6) + (sextet_c << 1 * 6) + (sextet_d << 0 * 6);

        if(j < *output_length)
            decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if(j < *output_length)
            decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if(j < *output_length)
            decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }

    return decoded_data;
}
