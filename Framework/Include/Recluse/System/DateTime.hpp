//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Arch.hpp"

namespace Recluse {

//////////////////////////////////////////////////////////
// Local date-time structure.
//////////////////////////////////////////////////////////
struct LocalDateTimeDesc
{
	U32 year;
	U32 month;
	U32 dayOfWeek;
	U32 day;
	U32 hour;
	U32 minute;
	U32 second;
	U32 milliseconds;
};


///////////////////////////////////////////////////////////
// 
// [OS SPECIFIC]
// Obtain the local time.
//
///////////////////////////////////////////////////////////
R_PUBLIC_API R_OS_CALL extern void getLocalDateTime(LocalDateTimeDesc& descOut);


class DateFormatter
{
	enum Date : U8
	{
		DATE_DAY = 0,
		DATE_MONTH,
		DATE_YEAR,
		DATE_SECOND,
		DATE_HOUR,
		DATE_MINUTE,
		DATE_COUNT = (DATE_MINUTE + 1)
	};

public:
	R_PUBLIC_API static const char kDay = 'D';
	R_PUBLIC_API static const char kMonth = 'M';
	R_PUBLIC_API static const char kYear = 'Y';
	R_PUBLIC_API static const char kHour = 'h';
	R_PUBLIC_API static const char kMinute = 'm';
	R_PUBLIC_API static const char kSecond = 's';
	R_PUBLIC_API static const char* kDefaultFormat;

	DateFormatter(const char* sFormat = kDefaultFormat)
	{
		LocalDateTimeDesc desc			= { };
		getLocalDateTime(desc);

		m_formatted = parseFormat(sFormat, desc);
	}

	DateFormatter(DateFormatter&& fmt)
	{
		m_formatted = std::move(fmt.m_formatted);
	}


	std::string getFormattedString() const
	{
		return m_formatted;
	}

private:
	static const char kDateTimeRegex = '%';

	std::string parseFormat(const char* cFormat, const LocalDateTimeDesc& desc) 
	{
		std::string fmt = cFormat;
		std::string formattedString = "";
		U32 fmtSize = fmt.size();
		U32 i = 0;
		for (; i < fmtSize; ++i)
		{
			if (fmt[i] == kDateTimeRegex && ((i+1) < fmtSize))
			{
				std::string str;
				if (fmt[i + 1] == kDay)
				{
					str = std::to_string(desc.day);
				}
				else if (fmt[i + 1] == kYear)
				{
					str = std::to_string(desc.year);
				}
				else if (fmt[i + 1] == kMonth)
				{
					str = std::to_string(desc.month);
				}
				else if (fmt[i + 1] == kHour)
				{
					str = std::to_string(desc.hour);
				}
				else if (fmt[i + 1] == kSecond)
				{
					str = std::to_string(desc.second);
				}
				else if (fmt[i + 1] == kMinute)
				{
					str = std::to_string(desc.minute);
				}

				formattedString += str;
				// move past the regex and date token.
				i += 1;
			}
			else
			{
				formattedString += fmt[i];
			}
		}
		return formattedString;
	}

	std::string m_formatted;
};
} // Recluse