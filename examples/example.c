#include <stdio.h>
#include <tws/tws.h>
#include <arpa/inet.h>

int main()
{
    char ip4[INET_ADDRSTRLEN];
    char ip6[INET6_ADDRSTRLEN];

    struct SockAddrIn sa;
    struct SockAddrInIPv6 sa6;

    inet_pton(AF_INET, "10.12.110.58", &(sa.addr));
    inet_pton(AF_INET6, "2001:db8:63b3:1::3490", &(sa6.addr));

    inet_ntop(AF_INET, &(sa.addr), ip4, INET_ADDRSTRLEN);
    printf("The IPv4 address is: %s\n", ip4);

    inet_ntop(AF_INET6, &(sa6.addr), ip6, INET6_ADDRSTRLEN);
    printf("The IPv6 address is: %s\n", ip6);
}
