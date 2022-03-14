#include "../../include/JsonFileReader.h"
#include "../../include/Logger.h"

namespace wlb
{
	using namespace Log;

	JsonFileReader::JsonFileReader(const std::string fileName)
	{
		std::ifstream ifile(fileName);
		if (!ifile.is_open())
		{
			exit(-1);
		}
		rapidjson::IStreamWrapper iSW(ifile);

		m_docData.ParseStream(iSW);
		if (m_docData.HasParseError())
		{
		}

		if (!m_docData.IsObject())
		{
		}

	}

	JsonFileReader::~JsonFileReader()
	{
	}






}