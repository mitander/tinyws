#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <tws/tws.h>

#include "include/sha1.h"
#include "include/base64.h"

int tws_handshake_accept(char *key, char **dst)
{
    struct sha_ctx ctx;
    char *str = malloc(sizeof(char) * (TWS_KEY_MS_LEN + 1));
    unsigned char hash[SHA1_HASH_SIZE];

    strcpy(str, key);
    strcat(str, MAGIC_STRING);

    sha1_reset(&ctx);
    sha1_input(&ctx, (const uint8_t *) str, TWS_KEY_MS_LEN);
    sha1_result(&ctx, hash);

    size_t size = SHA1_HASH_SIZE;
    *dst = base64_encode(hash, size, &size);
    free(str);
    return (0);
}

int tws_handshake_response(char *req, char **res)
{
    char *s;
    char *accept;

    for(s = strtok(req, "\r\n"); s != NULL; s = strtok(NULL, "\r\n"))
        if(strstr(s, TWS_HS_REQ) != NULL)
            break;

    s = strtok(s, " ");
    s = strtok(NULL, " ");

    tws_handshake_accept(s, &accept);

    *res = malloc(sizeof(char) * TWS_HS_ACCEPT_LEN);
    strcpy(*res, TWS_HS_ACCEPT);
    strcat(*res, (const char *) accept);
    strcat(*res, "\r\n\r\n");

    free(accept);
    return (0);
}
