





#include <iostream>
#include <string>
#include "univalue.h"

using namespace std;

int main (int argc, char *argv[])
{
    UniValue val;
    if (val.read(string(istreambuf_iterator<char>(cin),
                        istreambuf_iterator<char>()))) {
        cout << val.write(1 , 4 ) << endl;
        return 0;
    } else {
        cerr << "JSON Parse Error." << endl;
        return 1;
    }
}
