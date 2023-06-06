#include <iostream>
#include <signal.h>

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#include "message.h"

using namespace std;


int main(int argc, char **argv) {
    rapidjson::Document          obj;
    const char                  *wanted;
    decltype(obj.FindMember("")) it;
    std::string                  e;

    std::cout << std::string(msg, 184) << std::endl;

    //    JSONCHECKPARSE(data.data(), obj, false, e);
    do {
        //        obj.Parse((char *)data.data());
        if(obj.Parse(msg).HasParseError()) {
            e = std::string(rapidjson::GetParseError_En(obj.GetParseError()));
            return false;
        }
        if(!obj.IsObject()) {
            e = "parse error: is not a object.";
            return false;
        }
    } while(0);

    std::cout << "err : " << e << std::endl;

    return 0;
}
