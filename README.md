# tws
a tiny web socket library

# Usage

```c
// Called on opening connection to client
void open_cb(tws_client_t *client)
{
    char *addr;
    addr = tws_get_address(client->socket);
    printf("Connection opened. Client: %d - Addr: %s\n", client->socket, addr);
}

// Called on closing connection to client
void close_cb(tws_client_t *client)
{
    char *addr;
    addr = tws_get_address(client->socket);
    printf("Connection closed. Client: %d - Addr: %s\n", client->socket, addr);
}

// Called when receiving message from client
void msg_cb(tws_client_t *client, unsigned char *msg)
{
    char *addr;
    addr = tws_get_address(client->socket);
    printf("Received message from %s/%d: %s", addr, client->socket, msg);

    // Return received message to client
    tws_send_frame(client, msg);
    free(msg);
}

int main()
{
    // Create websocket server
    tws_server_t *ws = tws_server_init(8080);

    // Set callback functions
    ws->open_cb = &open_cb;
    ws->close_cb = &close_cb;
    ws->msg_cb = &msg_cb;

    // Listen on socket (blocking)
    tws_listen(ws);

    return 0;
}
```
