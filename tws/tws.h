#ifndef TWS_TWS_H
#define TWS_TWS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
#define TWS_EXTERN_C extern "C"
#else
#define TWS_EXTERN_C
#endif

#if defined(TWS_STATIC_LIBRARY)
#define TWS_EXPORT TWS_EXTERN_C
#else
#define TWS_EXPORT TWS_EXTERN_C __attribute__((visibility("default")))
#endif

struct InAddrIPv6
{
    unsigned char addr[16];
};

struct SockAddrInIPv6
{
    u_int16_t family;
    u_int16_t port;
    u_int32_t flowinfo;
    u_int32_t scope_id;
    struct InAddrIPv6 addr;
};
struct InAddr
{
    uint32_t addr;
};

struct SockAddrIn
{
    short int family;
    unsigned short int port;
    unsigned char zero[8];
    struct InAddr addr;
};

struct SockAddr
{
    unsigned short family;
    char data[14];
};

struct AddrInfo
{
    int flags;
    int family;
    int socktype;
    int protocol;
    size_t addrlen;
    char *chname;
    struct SockAddr *addr;
    struct AddrInfo *next;
};

struct SockAddrStorage
{
    int family; // TODO: check what type this should be.
};

#endif
