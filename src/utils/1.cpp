
#include <iostream>
// #define WS_DEBUG
#ifdef WS_DEBUG
#define COUT(x) std::cout << x
#define ENDL    << std::endl
#else
#define COUT(x) 
#define ENDL 
#endif

int main() {

    COUT(1<<2<<3) ENDL;
}