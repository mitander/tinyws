#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "tws/tws.h"

// global events
struct tws_events g_events;

int tws_send_frame(int fd, char *msg)
{
    unsigned char *res;
    unsigned char frame[10];
    uint8_t idx_data;
    uint64_t msg_len;
    int idx_res;
    int output;

    msg_len = strlen((char *) msg);
    frame[0] = (TWS_FIN | TWS_FRAME_OP_TXT);

    if(msg_len <= 125)
    {
        frame[1] = msg_len & 0x7F;
        idx_data = 2;
    }
    else if(msg_len >= 126 && msg_len <= 65535)
    {
        frame[1] = 126;
        frame[2] = (msg_len >> 8) & 255;
        frame[3] = msg_len & 255;
        idx_data = 4;
    }
    else
    {
        frame[1] = 127;
        frame[2] = (unsigned char) ((msg_len >> 56) & 255);
        frame[3] = (unsigned char) ((msg_len >> 48) & 255);
        frame[4] = (unsigned char) ((msg_len >> 40) & 255);
        frame[5] = (unsigned char) ((msg_len >> 32) & 255);
        frame[6] = (unsigned char) ((msg_len >> 24) & 255);
        frame[7] = (unsigned char) ((msg_len >> 16) & 255);
        frame[8] = (unsigned char) ((msg_len >> 8) & 255);
        frame[9] = (unsigned char) (msg_len & 255);
        idx_data = 10;
    }

    // frame bytes
    idx_res = 0;
    res = malloc(sizeof(unsigned char) * (idx_data + msg_len + 1));
    for(int i = 0; i < idx_data; i++)
    {
        res[i] = frame[i];
        idx_res++;
    }

    // data bytes
    for(int i = 0; i < msg_len; i++)
    {
        res[idx_res] = msg[i];
        idx_res++;
    }

    res[idx_res] = '\0';
    output = write(fd, res, idx_res);
    free(res);
    return output;
}

static unsigned char *tws_receive_frame(unsigned char *frame, size_t length, int *type)
{
    unsigned char *msg;
    uint8_t mask;
    uint8_t full_len;
    uint8_t idx_mask;
    uint8_t idx_data;
    size_t data_len;
    uint8_t masks[4];
    int i, j;

    msg = NULL;

    if(frame[0] == (TWS_FIN | TWS_FRAME_OP_TXT))
    {
        *type = TWS_FRAME_OP_TXT;
        idx_mask = 2;
        mask = frame[1];
        full_len = mask & 0x7F;

        if(full_len == 126)
        {
            idx_mask = 4;
        }
        else if(full_len == 127)
        {
            idx_mask = 10;
        }

        idx_data = idx_mask + 4;
        data_len = length - idx_data;

        masks[0] = frame[idx_mask + 0];
        masks[1] = frame[idx_mask + 1];
        masks[2] = frame[idx_mask + 2];
        masks[3] = frame[idx_mask + 3];

        msg = malloc(sizeof(unsigned char) * (data_len + 1));
        for(i = idx_data, j = 0; i < length; i++, j++)
        {
            msg[j] = frame[i] ^ masks[j % 4];
        }

        msg[j] = '\0';
    }
    else if(frame[0] == (TWS_FIN | TWS_FRAME_OP_CLOSE))
    {
        *type = TWS_FRAME_OP_CLOSE;
    }
    else
    {
        *type = frame[0] & 0x0F;
    }

    return msg;
}

char *tws_get_address(int sockfd)
{
    struct sockaddr_in addr;
    socklen_t addr_size;

    addr_size = sizeof(struct sockaddr_in);
    if(getpeername(sockfd, (struct sockaddr *) &addr, &addr_size) < 0)
    {
        return NULL;
    }

    return inet_ntoa(addr.sin_addr);
}

static void *tws_connect(void *vsock)
{
    int sock;
    size_t n;
    char *res;
    unsigned char frame[MSG_LEN];
    unsigned char *msg;
    int type;
    int hs_done;


    sock = (int) (intptr_t) vsock;

    while((n = read(sock, frame, sizeof(unsigned char) * MSG_LEN)) > 0)
    {
        if(!hs_done)
        {
            tws_handshake_response((char *) frame, &res);
            hs_done = 1;
            n = write(sock, res, strlen(res));
            g_events.open_cb(sock);
            free(res);
        }

        msg = tws_receive_frame(frame, n, &type);
        if(msg == NULL)
        {
            printf("Received invalid frame from client %d\n", sock);
        }

        if(type == TWS_FRAME_OP_TXT)
        {
            printf("Received text frame\n");
            g_events.msg_cb(sock, msg);
        }

        if(type == TWS_FRAME_OP_CLOSE)
        {
            printf("Received close frame: %d\n", type);
            g_events.close_cb(sock);
            goto closed;
        }
    }

closed:
    close(sock);
    return vsock;
}

int tws_socket_listen(struct tws_events *events, int port)
{
    int listen_sock;
    int sock;
    struct sockaddr_in server;
    struct sockaddr_in client;
    pthread_t client_thread;
    int len;

    len = sizeof(struct sockaddr_in);

    if(events == NULL)
    {
        printf("Events is NULL\n");
        exit(-1);
    }

    if(!events->close_cb || !events->msg_cb || !events->open_cb)
    {
        printf("Callback functions need to be set. close_cb=%d msg_cb=%d open_cb=%d\n",
               !(events->close_cb == NULL), !(events->msg_cb == NULL), !(events->open_cb == NULL));
        exit(-1);
    }

    if(port <= 0 || port > MAX_PORT)
    {
        printf("Invalid port: %d. Port must be in range 1-%d\n", port, MAX_PORT);
        exit(-1);
    }

    // copy events
    memcpy(&g_events, events, sizeof(struct tws_events));

    // create listen socket
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_sock < 0)
    {
        perror("Could not create socket\n");
        exit(-1);
    }

    // reuse previous address
    if(setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    {
        perror("Could not reuse address\n");
        exit(-1);
    }

    // setup sin
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    // bind socket
    if(bind(listen_sock, (struct sockaddr *) &server, sizeof(server)) < 0)
    {
        perror("Bind failed");
        exit(-1);
    }

    // listen to socket
    listen(listen_sock, MAX_CLIENTS);

    printf("Listening for incoming connections on port %d\n", port);

    // accept incoming connections
    while(1)
    {
        sock = accept(listen_sock, (struct sockaddr *) &client, (socklen_t *) &len);
        if(sock < 0)
        {
            perror("Could not accept connection\n");
            exit(-1);
        }

        // TODO: remove int to pointer cast
        if(pthread_create(&client_thread, NULL, tws_connect, (void *) (intptr_t) sock) != 0)
        {
            perror("Could not create client thread\n");
            exit(-1);
        }

        pthread_detach(client_thread);
    }
}
