#pragma once
#include<cstdio>
#include<iostream>
using namespace std;
void my_err(const char *str, int line)
{
    fprintf(stderr,"line:%d  ",line);
    perror(str);
    cout <<endl;
}