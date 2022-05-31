#include <stdio.h>
#include <tws/tws.h>

int main()
{
    tws_printy();
    int res = tws_divide(4);
    printf("res: %d\n", res);
}
