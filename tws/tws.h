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

#define TWS_HS_ACCEPT_LEN 130
#define TWS_HS_REQ "Sec-WebSocket-Key"
#define TWS_HS_ACCEPT                                                                              \
    "HTTP/1.1 101 Switching Protocols\r\n"                                                         \
    "Upgrade: websocket\r\n"                                                                       \
    "Connection: Upgrade\r\n"                                                                      \
    "Sec-WebSocket-Accept: "

#define TWS_MAX_CLIENTS 8
#define TWS_MAX_PORT 65535

#define TWS_MAGIC_STRING "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define TWS_MSG_LEN 2048

#define TWS_FIN 128
#define TWS_FRAME_OP_TXT 1
#define TWS_FRAME_OP_CLOSE 8
#define TWS_FRAME_OP_UNSUPPORTED 0xF
#define TWS_FRAME_MAX_LEN (16 * 1024 * 1024)

#define TWS_KEY_LEN 24
#define TWS_MS_LEN 36
#define TWS_KEY_MS_LEN (TWS_KEY_LEN + TWS_MS_LEN)


typedef struct tws_client tws_client_t;
typedef struct tws_server tws_server_t;

struct tws_client
{
    tws_server_t *ws;
    tws_client_t *prev;
    tws_client_t *next;

    int id;
    int socket;
    int server_socket;
    int address;
    char *ws_key;
    void *data;
};

struct tws_server
{
    int port;
    tws_client_t *current;
    tws_client_t *clients;

    void (*open_cb)(int);
    void (*close_cb)(int);
    void (*msg_cb)(int, unsigned char *);
};


TWS_EXPORT tws_server_t *tws_server_init(int port);
TWS_EXPORT tws_client_t *tws_client_init(void);

TWS_EXPORT int tws_handshake_accept(char *key, char **dst);
TWS_EXPORT int tws_handshake_response(char *req, char **res);

TWS_EXPORT int tws_send_frame(int fd, char *msg);
TWS_EXPORT int tws_listen(tws_server_t *socket);
TWS_EXPORT char *tws_get_address(int fd);

#endif // TWS_TWS_H
