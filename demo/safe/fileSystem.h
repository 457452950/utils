#ifndef FILE_SYS
#define FILE_SYS


#include <iostream>
#include<filesystem>

using namespace std;
using namespace std::filesystem;


int test(){
	path str("/home/ubuntu");
	if (!exists(str)) {		//必须先检测目录是否存在才能使用文件入口.
		cout << "not exists" << endl;
        return 1;
    }
    
	directory_entry entry(str);		//文件入口
	if (entry.status().type() == file_type::directory)	//这里用了C++11的强枚举类型
		cout << "该路径是一个目录" << endl;

	directory_iterator list(str);	        //文件入口容器
	for (auto& it:list) 
		cout << it.path().filename()<< endl;	//通过文件入口（it）获取path对象，再得到path对象的文件名，将之输出
	
    
	return 0;
}

#endif // FILE_SYS