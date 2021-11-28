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
			LOG(ERROR) << "[JsonFileReader::JsonFileReader]!ifile.is_open():";
			exit(-1);
		}
		rapidjson::IStreamWrapper iSW(ifile);

		m_docData.ParseStream(iSW);
		if (m_docData.HasParseError())
		{
			LOG(ERROR) << "[JsonFileReader::JsonFileReader]HasParseError:" << m_docData.GetParseError();
		}

		if (!m_docData.IsObject())
		{
			LOG(ERROR) << "[JsonFileReader::JsonFileReader]IsObject:flase";
		}

	}

	JsonFileReader::~JsonFileReader()
	{
	}






}