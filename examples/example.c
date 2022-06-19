#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <tws/tws.h>

void open_cb(int fd)
{
    char *addr;
    addr = tws_get_address(fd);
    printf("Connection opened. Client: %d - Addr: %s\n", fd, addr);
}

void close_cb(int fd)
{
    char *addr;

    addr = tws_get_address(fd);
    printf("Connection closed. Client: %d - Addr: %s\n", fd, addr);
}

void msg_cb(int fd, unsigned char *msg)
{
    char *addr;

    addr = tws_get_address(fd);
    printf("Received message from %s/%d: %s\n", addr, fd, msg);

    tws_send_frame(fd, (char *) msg);

    free(msg);
}

int main()
{
    tws_server *ws = tws_server_init(8080);
    ws->open_cb = &open_cb;
    ws->close_cb = &close_cb;
    ws->msg_cb = &msg_cb;
    tws_listen(ws);

    return 0;
}
