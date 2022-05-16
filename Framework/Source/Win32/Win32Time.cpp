//
#include "Win32/Win32Common.hpp"
#include "Recluse/Time.hpp"
#include "Recluse/System/DateTime.hpp"

namespace Recluse {


void getLocalDateTime(LocalDateTimeDesc& desc)
{
	SYSTEMTIME systemTime	= { };

	GetLocalTime(&systemTime);

	desc.day			= systemTime.wDay;
	desc.dayOfWeek		= systemTime.wDayOfWeek;
	desc.hour			= systemTime.wHour;
	desc.milliseconds	= systemTime.wMilliseconds;
	desc.minute			= systemTime.wMinute;
	desc.month			= systemTime.wMonth;
	desc.second			= systemTime.wSecond;
	desc.year			= systemTime.wYear;
}
} // Recluse