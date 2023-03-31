#include <iostream>
#include <chrono>
#include "rsa.h"

using namespace std;

void showint(unsigned short *value, int len) {
    for(int i = 0; i < len; ++i)
        cout << value[i];
}

int main() {

    cout << "Hello safety";

    mpuint d(2);
    mpuint e(2);
    mpuint n(2);

    GenerateKeys(d, e, n);
    d.dump();
    cout << " -- ";
    e.dump();
    cout << " -- ";
    n.dump();
    cout << "\n";

    char   str1[] = "45185648256151256122268646541856484864564867486486486486486748648678648674864867684867864165486486"
                  "45185648256151256122268646541856484864564867486486486486486748648678648674864867684867864165486486"
                  "45185648256151256122268646541856484864564867486486486486486748648678648674864867684867864165486486"
                  "45185648256151256122268646541856484864564867486486486486486748648678648674864867684867864165486486"
                  "45185648256151256122268646541856484864564867486486486486486748648678648674864867684867864165486486"
                  "45185648256151256122268646541856484864564867486486486486486748648678648674864867684867864165486486"
                  "45185648256151256122268646541856484864564867486486486486486748648678648674864867684867864165486486"
                  "45185648256151256122268646541856484864564867486486486486486748648678648674864867684867864165486486"
                  "45185648256151256122268646541856484864564867486486486486486748648678648674864867684867864165486486"
                  "45185648256151256122268646541856484864564867486486486486486748648678648674864867684867864165486486";
    char str2[4096] = {0};
    const char *p = str1;
    char *q = str2;

    mpuint srv_result(2);
    mpuint srv_source(2);
    mpuint cus_result(2);
    mpuint cus_source(2);

    auto now =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
                    .count();
    while(p - str1 != strlen(str1)) {
        srv_source.scan(p);
        EncryptDecrypt(srv_result, srv_source, e, n);
        // result = source ^ d % n
        cus_source = srv_result;
        EncryptDecrypt(cus_result, cus_source, d, n);
        cus_result.edit(q);
        q += 4;
    }
    std::cout << "pass "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count() - now << endl;
    cout << str2 << endl << strcmp(str1, str2) << endl;

    //srv_source.dump();
    //cout << " --> ";
    //srv_result.dump();
    //cout << "\n";


    //cus_source = srv_result;
    //now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
    //              .count();
    //// result = source ^ d % n
    //EncryptDecrypt(cus_result, cus_source, d, n);
    //std::cout << "pass "
    //          << std::chrono::duration_cast<std::chrono::milliseconds>(
    //                     std::chrono::system_clock::now().time_since_epoch())
    //                        .count() -
    //                now
    //          << endl;
    //cus_result.edit(str2);
    //cout << "str2:" << str2 << endl;
    //cus_source.dump();
    //cout << " --> ";
    //cus_result.dump();
    //cout << "\n";

    system("pause");
    return 0;
}