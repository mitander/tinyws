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
    free(addr);
}

void close_cb(int fd)
{
    char *addr;
    addr = tws_get_address(fd);
    printf("Connection closed. Client: %d - Addr: %s\n", fd, addr);
    free(addr);
}

void msg_cb(int fd, unsigned char *msg)
{
    char *addr;
    addr = tws_get_address(fd);
    printf("Received message from %s/%d: %s\n", addr, fd, msg);

    tws_send_frame(fd, (char *) msg);

    free(addr);
    free(msg);
}

int main()
{
    struct tws_events evs;
    evs.open_cb = &open_cb;
    evs.close_cb = &close_cb;
    evs.msg_cb = &msg_cb;
    tws_socket_listen(&evs, 8080);

    return 0;
}
