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

#define MSG_LEN 2048

#define MAX_CLIENTS 8
#define MAX_PORT 65535

#define TWS_KEY_LEN 24
#define TWS_MS_LEN 36
#define TWS_KEY_MS_LEN (TWS_KEY_LEN + TWS_MS_LEN)
#define MAGIC_STRING "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

#define TWS_HS_REQ "Sec-WebSocket-Key"
#define TWS_HS_ACCEPT_LEN 130
#define TWS_HS_ACCEPT                                                                              \
    "HTTP/1.1 101 Switching Protocols\r\n"                                                         \
    "Upgrade: websocket\r\n"                                                                       \
    "Connection: Upgrade\r\n"                                                                      \
    "Sec-WebSocket-Accept: "

#define TWS_FIN 128
#define TWS_FRAME_OP_TXT 1
#define TWS_FRAME_OP_CLOSE 8
#define TWS_FRAME_OP_UNSUPPORTED 0xF
#define TWS_FRAME_MAX_LEN (16 * 1024 * 1024)

typedef struct tws_client tws_client;
typedef struct tws_socket tws_socket;

struct tws_client
{
    tws_socket *ws;
    tws_client *prev;
    tws_client *next;

    int id;
    int socket;
    int server_socket;
    int address;
    char *ws_key;
    void *data;
};

struct tws_socket
{
    int port;
    tws_client *current;
    tws_client *clients;

    void (*open_cb)(int);
    void (*close_cb)(int);
    void (*msg_cb)(int, unsigned char *);
};


TWS_EXPORT tws_socket *tws_socket_init(int port);
TWS_EXPORT tws_client *tws_client_init(void);

TWS_EXPORT int tws_handshake_accept(char *key, char **dst);
TWS_EXPORT int tws_handshake_response(char *req, char **res);

TWS_EXPORT int tws_send_frame(int fd, char *msg);
TWS_EXPORT int tws_socket_listen(tws_socket *socket);
TWS_EXPORT char *tws_get_address(int fd);

#endif // TWS_TWS_H
