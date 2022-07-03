#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <tws/tws.h>

void open_cb(tws_client_t *client)
{
    char *addr;
    addr = tws_get_address(client->socket);
    printf("Connection opened. Client: %d - Addr: %s\n", client->socket, addr);
}

void close_cb(tws_client_t *client)
{
    char *addr;
    addr = tws_get_address(client->socket);
    printf("Connection closed. Client: %d - Addr: %s\n", client->socket, addr);
}

void msg_cb(tws_client_t *client, unsigned char *msg)
{
    char *addr;
    addr = tws_get_address(client->socket);
    printf("Received message from %s/%d: %s", addr, client->socket, msg);

    tws_send_frame(client, msg);
    free(msg);
}

int main()
{
    tws_server_t *ws = tws_server_init(8080);
    ws->open_cb = &open_cb;
    ws->close_cb = &close_cb;
    ws->msg_cb = &msg_cb;
    tws_listen(ws);

    return 0;
}
