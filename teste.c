#include <stdlib.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
    char *teste = "0x3C000000";
    int kkk = strtol(teste,NULL,0);
    printf("%d",kkk);
    return 0;
}
