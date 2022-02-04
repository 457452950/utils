#pragma once
#ifndef __JSONFILEREADER
#define __JSONFILEREADER

#include <iostream>
#include <fstream>
#include <string>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>

namespace wlb
{

class JsonFileReader
{
public:
	JsonFileReader(const std::string fileName);
	~JsonFileReader();

	
    rapidjson::Value& operator[](std::string key){
        return  m_docData[key.c_str()];
    }
    rapidjson::Value& operator[](int key){
        return  m_docData[key];
    }
    
    bool HasMember(std::string key){
        return m_docData.HasMember(key.c_str());
    }

    auto FindMember(const std::string& key){
        return m_docData.FindMember(key.c_str());
    }
    auto MemberEnd(){
        return m_docData.MemberEnd();
    }
    
private:
	rapidjson::Document m_docData;
};








}



#endif // !__JSONFILEREADER




