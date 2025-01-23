#include "Globals.h"
#include "Core/Exceptions.h"
#include "Core/Logging.h"
#include "Core/Profiling.h"

namespace X
{
	CGlobals* pGlobals = 0;

	CGlobals::CGlobals()
	{
		pLog = 0;
		pProfiler = 0;
	}

	CGlobals::~CGlobals()
	{
		if (pProfiler)
		{
			delete pProfiler;
			pProfiler = 0;
		}
		if (pLog)
		{
			delete pLog;
			pLog = 0;
		}
	}

	void CGlobals::init(void)
	{
		// Other classes' constructors may throw exceptions, so we need to create the log object first.
		pLog = new CLog;
		ThrowIfMemoryNotAllocated(pLog);

		pProfiler = new CProfiler;
		ThrowIfMemoryNotAllocated(pProfiler);

		LOG("Log entry example.");
		LOGVERBOSE("Log verbose entry example.");
		LOGERROR("Log error entry example.");

		// Write system information after the heading, but before the main table begins
		// RAM usage of process
		LOGHEADING("System Information...");
		double dGBSystemTotal, dGBSystemAvailable, dGBSystemUsed, dGBUsedByProcess;
		getMemoryInfo(dGBSystemTotal, dGBSystemAvailable, dGBSystemUsed, dGBUsedByProcess);
		std::string strInfo;
		strInfo = "RAM total on system: " + std::to_string(dGBSystemTotal) + "GB";				LOG(strInfo);
		strInfo = "RAM available on system: " + std::to_string(dGBSystemAvailable) + "GB";		LOG(strInfo);
		strInfo = "RAM used by process: " + std::to_string(dGBUsedByProcess) + "GB";			LOG(strInfo);
		strInfo = "Number of logical CPU cores: " + std::to_string(getCPULogicalCoresCount());	LOG(strInfo);
		LOGHEADING("System Information End.");

	}
}