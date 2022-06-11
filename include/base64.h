#ifndef TWS_BASE64_H
#define TWS_BASE64_H

#include <sys/types.h>

unsigned char *base64_encode(const unsigned char *src, size_t len, size_t *out_len);
unsigned char *base64_decode(const unsigned char *src, size_t len, size_t *out_len);

#endif // BASE64_H
