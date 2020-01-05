// #include "typetools.h"
#include <iostream>
#include <string>
#include <map>
#include <cmath>
using namespace std;

/*struct a {
    friend a operator+(const a &p, const a &q) {
        a res = p;
        res.val += q.val;
        return res;
    }
    a(int d) { val = d; }
    int val = 0;
};*/

int main() {
//     alltype a(false), b(738492847), c;
//     cout << c._type <<endl;
//     c *= 4;
//     cout << c.toString();
//     cout << a.toString();
    // Bigint a("1"), b("-7"), c(-0);
    // cout << (string)(a / b) << endl;
    // cout <<(string)c <<endl;
    double a = 23.77568476934;
    cout << (int)a;
    return 0;
}