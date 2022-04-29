#include "WSystem.h"

namespace wlb
{

bool mkdir(const std::string& path)
{
#if OS_IS_WINDOWS
    ::mkdir(path.c_str());
#elif OS_IS_LINUX
    ::mkdir(path.c_str(), 477);
#endif
}


bool IsFileExist(const std::string& path)
{
    return !access(path.c_str(), 0);
}






}

