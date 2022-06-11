#ifndef TWS_BASE64_H
#define TWS_BASE64_H

#include <sys/types.h>

char *base64_encode(const unsigned char *data, size_t len);
char *base64_decode(const unsigned char *data, size_t len, size_t *out_len);

#endif // TWS_BASE64_H
