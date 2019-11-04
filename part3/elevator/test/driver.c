#include <stdlib.h>
#include <stdio.h>
#include "wrappers.h"

int main(int argc, char **argv)
{

    if (argc != 1)      { printf("Too many arguments lol\n"); return -1; }
    int i,j;
    
    for (i = 0; i < 3; i++)
    {
        issue_request(1, i, (i+1)*2);
        issue_request(2, i, (i+1)*2);
        issue_request(3, i, (i+1)*2);
        issue_request(4, i, (i+1)*2);
    }

    return 0;
}
