#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <tws/tws.h>

#include "include/sha1.h"
#include "include/base64.h"

int tws_handshake_accept(char *key, char **dst)
{
    struct sha_ctx ctx;
    unsigned char hash[SHA1_HASH_SIZE];
    char *str;
    int err;

    if(!key)
    {
        return -1;
    }

    str = calloc(1, sizeof(char) * (TWS_KEY_MS_LEN + 1));
    if(!str)
    {
        return -1;
    }

    strncpy(str, key, TWS_KEY_LEN);
    strcat(str, TWS_MAGIC_STRING);

    if((err = sha1_reset(&ctx)))
    {
        return -1;
    }

    if((err = sha1_input(&ctx, (const uint8_t *) str, TWS_KEY_MS_LEN)))
    {
        return -1;
    }

    if((err = sha1_result(&ctx, hash)))
    {
        return -1;
    }

    *dst = base64_encode(hash, 20);
    *(*dst + strlen((const char *) *dst) - 1) = '\0';

    free(str);
    return 0;
}

int tws_handshake_response(char *req, char **res)
{
    char *accept;
    char *saveptr;
    char *str;

    saveptr = NULL;
    for(str = strtok_r(req, "\r\n", &saveptr); str != NULL; str = strtok_r(NULL, "\r\n", &saveptr))
    {
        if(strstr(str, TWS_HS_REQ) != NULL)
        {
            break;
        }
    }

    if(str == NULL)
    {
        printf("str is null\n");
        return -1;
    }

    saveptr = NULL;
    str = strtok_r(str, " ", &saveptr);
    str = strtok_r(NULL, " ", &saveptr);

    if(tws_handshake_accept(str, &accept) != 0)
    {
        return -1;
    }

    *res = malloc(sizeof(char) * TWS_HS_ACCEPT_LEN);
    if(*res == NULL)
    {
        return -1;
    }

    strcpy(*res, TWS_HS_ACCEPT);
    strcat(*res, (const char *) accept);
    strcat(*res, "\r\n\r\n");

    free(accept);
    return 0;
}
