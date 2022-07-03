#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#include "tws/tws.h"

unsigned long long clients_id = 0;

tws_server_t *tws_server_init(int port)
{
    tws_server_t *socket = malloc(sizeof(tws_server_t));
    socket->port = port;
    socket->open_cb = NULL;
    socket->msg_cb = NULL;
    socket->close_cb = NULL;
    socket->clients = NULL;
    socket->current = NULL;

    return socket;
}

tws_client_t *tws_client_init()
{
    tws_client_t *client = malloc(sizeof(tws_client_t));
    client->socket = 0;
    client->ws_key = NULL;
    client->address = 0;
    client->data = NULL;
    client->next = NULL;
    client->prev = NULL;

    return client;
}

int tws_send_frame(tws_client_t *client, unsigned char *msg)
{
    unsigned char *res;
    unsigned char frame[10];
    uint8_t idx_data;
    uint64_t msg_len;
    int idx_res;

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

    write(client->socket, res, idx_res);

    free(res);

    return 0;
}

static unsigned char *tws_receive_frame(unsigned char *frame, size_t len, int *type)
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

    printf("Frame length: %lu\n", strlen((char *) frame));

    switch(frame[0])
    {
        case(TWS_FIN | TWS_FRAME_OP_TXT):
            *type = TWS_FRAME_OP_TXT;
            mask = frame[1];
            full_len = mask & 0x7F;


            switch(full_len)
            {
                case 126:
                    idx_mask = 4;
                case 127:
                    idx_mask = 10;
                default:
                    idx_mask = 2;
            }

            idx_data = idx_mask + 4;
            data_len = len - idx_data;

            masks[0] = frame[idx_mask + 0];
            masks[1] = frame[idx_mask + 1];
            masks[2] = frame[idx_mask + 2];
            masks[3] = frame[idx_mask + 3];

            msg = malloc(sizeof(unsigned char) * (data_len + 1));
            for(i = idx_data, j = 0; i < len; i++, j++)
            {
                msg[j] = frame[i] ^ masks[j % 4];
            }

            msg[j] = '\0';

        case(TWS_FIN | TWS_FRAME_OP_CLOSE):
            *type = TWS_FRAME_OP_CLOSE;

        default:
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
        printf("Could not get peer name");
        return NULL;
    }

    return inet_ntoa(addr.sin_addr);
}

static void *tws_connect(tws_client_t *client)
{
    size_t n;
    char *res;
    unsigned char frame[TWS_MSG_LEN];
    unsigned char *msg;
    int type;
    int hs_done;
    int sock;
    char str_addr[INET6_ADDRSTRLEN + 1];

    inet_ntop(AF_INET, (void *) &client->address, str_addr, INET_ADDRSTRLEN);

    if(client == NULL)
    {
        printf("Client is null\n");
        return NULL;
    }

    if(client->ws == NULL)
    {
        printf("Client ws is null\n");
        return NULL;
    }

    sock = (int) (intptr_t) client->socket;

    while((n = read(sock, frame, sizeof(frame))) > 0)
    {
        if(!hs_done)
        {
            if(tws_handshake_response((char *) frame, &res) != 0)
            {
                printf("Handshake reponse failed\n");
                return NULL;
            }

            hs_done = 1;
            n = write(sock, res, strlen(res));
            client->ws->open_cb(client);
            free(res);
        }

        if((msg = tws_receive_frame(frame, n, &type)) == NULL)
        {
            printf("Invalid frame from client %d\n", sock);
        }

        if(type == TWS_FRAME_OP_TXT)
        {
            printf("Text frame\n");
            client->ws->msg_cb(client, msg);
        }

        if(type == TWS_FRAME_OP_CLOSE)
        {
            printf("Close frame: %d\n", type);
            client->ws->close_cb(client);
            goto closed;
        }
    }

closed:
    close(sock);
    return NULL;
}


static void broken_pipe_handler(int sig)
{
    printf("Broken pipe.\n");
}

static void *client_handler(void *client)
{
    sigaction(SIGPIPE, &(struct sigaction){broken_pipe_handler}, NULL);
    tws_connect(client);
    return NULL;
}

int tws_listen(tws_server_t *server)
{
    int server_sock;
    int client_sock;
    int client_len;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    pthread_t client_thread;
    char addr_len[INET6_ADDRSTRLEN + 1];

    sigaction(SIGPIPE, &(struct sigaction){broken_pipe_handler}, NULL);

    if(server == NULL)
    {
        printf("Server is null\n");
        exit(-1);
    }

    if(!server->close_cb || !server->msg_cb || !server->open_cb)
    {
        printf("Callback functions need to be set. close_cb=%d msg_cb=%d open_cb=%d\n",
               !(server->close_cb == NULL), !(server->msg_cb == NULL), !(server->open_cb == NULL));
        exit(-1);
    }

    if(server->port <= 0 || server->port > TWS_MAX_PORT)
    {
        printf("Invalid port: %d. Port must be in range 1-%d\n", server->port, TWS_MAX_PORT);
        exit(-1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(server->port);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sock < 0)
    {
        perror("Could not create socket\n");
        exit(-1);
    }

    if(setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    {
        perror("Could not reuse address\n");
        exit(-1);
    }


    if(bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        exit(-1);
    }

    listen(server_sock, TWS_MAX_CLIENTS);
    client_len = sizeof(client_addr);

    printf("Listening for incoming connections on port %d\n", server->port);

    for(;;)
    {
        client_sock =
          accept(server_sock, (struct sockaddr *) &client_addr, (socklen_t *) &client_len);
        if(client_sock < 0)
        {
            perror("Could not accept connection\n");
            exit(-1);
        }

        inet_ntop(AF_INET, (void *) &client_addr.sin_addr, addr_len, INET_ADDRSTRLEN);

        tws_client_t *client = tws_client_init();
        client->id = ++clients_id;
        client->ws = server;
        client->server_socket = server_sock;
        client->socket = client_sock;
        client->address = client_addr.sin_addr.s_addr;

        if(!server->clients)
        {
            server->clients = client;
        }
        else
        {
            client->prev = server->current;
            server->current->next = client;
        }

        server->current = client;
        server->open_cb(client);

        printf("Client connected: #%d (%s)\n", client->id, addr_len);

        if(pthread_create(&client_thread, NULL, &client_handler, (void *) client))
        {
            perror("Could not create client thread\n");
            exit(-1);
        }

        pthread_detach(client_thread);
    }
}
