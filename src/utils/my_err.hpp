#include<cstdio>
void my_err(char * str,int line)
{
    fprintf(stderr,"line:%d",line);
    perror(str);
}