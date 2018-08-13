#include <functional>
#include <algorithm>
#include <iostream>

using namespace std;
namespace p = std::placeholders;

int f(int a, int b, int c)
{
    return a + b + c;
}

int main()
{
    auto x = bind(f, 1, p::_1, p::_2);
    auto y = x;
    // cout << x(2, 3) << endl; // 

    // cout <<([x] {
    //     x(2, 3);
    // } () )<< endl;
}