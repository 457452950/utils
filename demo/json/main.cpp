#include <iostream>

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#include "message.h"

using namespace std;


int main(int argc, char **argv) {
    rapidjson::Document obj;
    std::string         e;

    std::cout << std::string(msg, std::size(msg)) << std::endl;

    if(obj.Parse(msg, std::size(msg)).HasParseError()) {
        e = std::string(rapidjson::GetParseError_En(obj.GetParseError()));
    } else {
        if(!obj.IsObject()) {
            e = "parse error: is not a object.";
        }
    }

    std::cout << "err : " << e << std::endl;

    return 0;
}
