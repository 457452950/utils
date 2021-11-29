#include "../../include/JsonFileReader.h"
#include "../../include/Logger.h"

namespace wlb
{
	using namespace Log;

	JsonFileReader::JsonFileReader(std::string fileName)
	{
		std::ifstream ifile(fileName);
		if (!ifile.is_open())
		{
			LOG(L_ERROR) << "[JsonFileReader::JsonFileReader]!ifile.is_open():";
			exit(-1);
		}
		rapidjson::IStreamWrapper iSW(ifile);

		m_docData.ParseStream(iSW);
		if (m_docData.HasParseError())
		{
			LOG(L_ERROR) << "[JsonFileReader::JsonFileReader]HasParseError:" << m_docData.GetParseError();
		}

		if (!m_docData.IsObject())
		{
			LOG(L_ERROR) << "[JsonFileReader::JsonFileReader]IsObject:flase";
		}

	}

	JsonFileReader::~JsonFileReader()
	{
	}






}