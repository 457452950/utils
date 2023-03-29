#include "rsa.h"
#include <iostream>

using namespace std;

void showint(unsigned short *value, int len) {
    for(int i = 0; i < len; ++i)
        cout << value[i];
}

int main() {

    cout << "Hello safety";

    mpuint d(5);
    mpuint e(5);
    mpuint n(5);

    GenerateKeys(d, e, n);
    showint(d.value, d.length);
    cout << " -- ";
    showint(e.value, e.length);
    cout << " -- ";
    showint(n.value, n.length);
    cout << "\n";

    mpuint srv_result(10);
    mpuint srv_source(10);
    mpuint cus_result(10);
    mpuint cus_source(10);

    const char *str = "4634";
    srv_source.scan(str);
    EncryptDecrypt(srv_result, srv_source, e, n);
    showint(srv_source.value, srv_source.length);
    cout << " --> ";
    showint(srv_result.value, srv_result.length);
    cout << "\n";

    cus_source = srv_result;
    EncryptDecrypt(cus_result, cus_source, d, n);
    showint(cus_source.value, cus_source.length);
    cout << " --> ";
    showint(cus_result.value, cus_result.length);
    cout << "\n";

    system("pause");
    return 0;
}