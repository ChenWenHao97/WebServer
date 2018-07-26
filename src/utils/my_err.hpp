#pragma once
#include<cstdio>
void my_err(const char *str, int line)
{
    fprintf(stderr,"line:%d",line);
    perror(str);
}