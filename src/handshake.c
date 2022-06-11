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

    if(!key)
        // invalid key
        return -1;

    str = calloc(1, sizeof(char) * (TWS_KEY_LEN + TWS_MS_LEN + 1));
    if(!str)
        return -1;

    strncpy(str, key, TWS_KEY_LEN);
    strcat(str, MAGIC_STRING);

    sha1_reset(&ctx);
    sha1_input(&ctx, (const uint8_t *) str, TWS_KEY_MS_LEN);
    sha1_result(&ctx, hash);

    *dst = base64_encode(hash, 20);
    *(*dst + strlen((const char *) *dst) - 1) = '\0';
    free(str);
    return (0);
}

int tws_handshake_response(char *req, char **res)
{
    char *accept;
    char *saveptr;
    char *s;
    int ret;

    saveptr = NULL;
    for(s = strtok_r(req, "\r\n", &saveptr); s != NULL; s = strtok_r(NULL, "\r\n", &saveptr))
    {
        if(strstr(s, TWS_HS_REQ) != NULL)
            break;
    }

    if(s == NULL)
        return (-1);

    saveptr = NULL;
    s = strtok_r(s, " ", &saveptr);
    s = strtok_r(NULL, " ", &saveptr);

    ret = tws_handshake_accept(s, &accept);
    if(ret < 0)
        return (ret);

    *res = malloc(sizeof(char) * TWS_HS_ACCEPT_LEN);
    if(*res == NULL)
        return (-1);

    strcpy(*res, TWS_HS_ACCEPT);
    strcat(*res, (const char *) accept);
    strcat(*res, "\r\n\r\n");


    free(accept);
    return 0;
}
