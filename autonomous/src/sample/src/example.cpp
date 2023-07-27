//
// Created by Muralidhar Ravuri on 10/26/18.
//

#include <ATen/ATen.h>
#include <iostream>

using namespace at;
using namespace std;

int main(int argc, char** argv) {

    Tensor a = ones({2, 2}, kInt);
    Tensor b = randn({2, 2});

    auto c = a + b.to(kInt);

    cout << b << endl;
    cout << c << endl;

    return 0;
}
