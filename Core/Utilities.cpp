#include "Utilities.h"
#include "Logging.h"
#include <algorithm>
#include <filesystem>
#include <thread>	// For std::thread::hardware_concurrency();
#include "Exceptions.h"

#ifdef PLATFORM_WINDOWS
#include <direct.h>				// For _chdir on Windows
#include <pdh.h>
#include <pdhmsg.h>
#define NOMINMAX				// Set this before including windows.h so that the min/max macros located in algorithm header take precedence
#define WIN32_LEAN_AND_MEAN		// Exclude rarely used stuff from Windows headers
#include <Windows.h>
#include <psapi.h>
#pragma comment(lib, "Pdh.lib")
//#include <direct.h>
#elif PLATFORM_LINUX
#include <unistd.h>				// For chdir on Linux
#endif

namespace X
{
	void getHueColour(float fHueAmount, float& fRed, float& fGreen, float& fBlue)
	{
		clamp(fHueAmount, 0.0f, 1.0f);
		fHueAmount *= 360.0f;
		if (fHueAmount <= 60.0f)	// Inc green
		{
			fRed = 1.0f;
			fGreen = fHueAmount / 60.0f;
			fBlue = 0.0f;
		}
		else if (fHueAmount <= 120.0f)	// Dec red
		{
			fRed = 1.0f - ((fHueAmount - 60.0f) / 60.0f);
			fGreen = 1.0f;
			fBlue = 0.0f;
		}
		else if (fHueAmount <= 180.0f)	// Inc blue
		{
			fRed = 0.0f;
			fGreen = 1.0f;
			fBlue = (fHueAmount - 120.0f) / 60.0f;
		}
		else if (fHueAmount <= 240.0f)	// Dec green
		{
			fRed = 0.0f;
			fGreen = 1.0f - ((fHueAmount - 180.0f) / 60.0f);
			fBlue = 1.0f;
		}
		else if (fHueAmount <= 300.0f)	// Inc red
		{
			fRed = (fHueAmount - 240.0f) / 60.0f;
			fGreen = 0.0f;
			fBlue = 1.0f;
		}
		else // dec blue
		{
			fRed = 1.0f;
			fGreen = 0.0f;
			fBlue = 1.0f - ((fHueAmount - 300.0f) / 60.0f);
		}
	}

	bool convertFileToHeader(const std::string& strFilename, const std::string& strArrayName, unsigned int uiNumElementsPerRow)
	{
		FILE* fs = NULL;  // Source file
		FILE* fd = NULL;  // Destination file

		std::string strFilenameInput = strFilename;
		std::string strFilenameOutput = strFilename;
		strFilenameOutput.append(".h");

		// Open source
		fopen_s(&fs, strFilename.c_str(), "rb");
		if (fs == NULL)
			return false;

		// Open destination
		fopen_s(&fd, strFilenameOutput.c_str(), "wb");
		if (fd == NULL)
		{
			fclose(fs);
			return false;
		}

		// Write comment giving name of binary file that this data came from
		fprintf(fd, "// Following data taken from file \"%s\"\n", strFilename.c_str());

		// Get length of data
		fseek(fs, 0, SEEK_END);
		int length = ftell(fs);
		fseek(fs, 0, SEEK_SET);

		// Write #define for length of array
		std::string strArrayNameUppercase = strArrayName;
		std::transform(strArrayNameUppercase.begin(), strArrayNameUppercase.end(), strArrayNameUppercase.begin(), ::toupper);
		strArrayNameUppercase.append("_SIZE");
		fprintf(fd, "#define %s %d\n", strArrayNameUppercase.c_str(), length);

		// Write start of array
		fprintf(fd, "unsigned char %s[] =\n", strArrayName.c_str());
		fprintf(fd, "{\n ");

		// Write out data from source binary file
		while (feof(fs) == false)
		{
			unsigned char tmp;
			for (int e = 0; e < (int)uiNumElementsPerRow; e++)
			{
				fread(&tmp, 1, sizeof(unsigned char), fs);
				if (feof(fs) == false)
					fprintf(fd, "0x%02x,", tmp);
			}

			if (feof(fs)) // Reached end of file
			{
				fseek(fd, -1, SEEK_CUR);  // Remove last comma
				fprintf(fd, "};");
			}
			fprintf(fd, "\n ");
		}
		fprintf(fd, "\n");

		fclose(fs);
		fclose(fd);
		return true;
	}

	bool getFileExists(const std::string& strFilename)
	{
		/*
		FILE* f;
		if (fopen_s(&f, strFilename.c_str(), "rb"))
			return false;
		fclose(f);
		return true;
		*/
		bool bExists = false;
		std::ifstream file(strFilename);
		if (file.is_open())
		{
			bExists = true;
			file.close();
		}
		return bExists;
	}

	bool deleteFile(const std::string& strFilenameToDelete)
	{
		std::remove(strFilenameToDelete.c_str());
		return !getFileExists(strFilenameToDelete);
	}

	bool renameFile(const std::string& strOldFilename, const std::string& strNewFilename)
	{
		if (0 == std::rename(strOldFilename.c_str(), strNewFilename.c_str()))
			return true;
		return false;
	}

	std::string getCurrentDirectory(void)
	{
		return std::filesystem::current_path().string();
	}

	bool setCurrentDirectory(const std::string& strPathOfNewCurrentDirectory)
	{
#ifdef PLATFORM_WINDOWS
		return 0 == _chdir(strPathOfNewCurrentDirectory.c_str());
#elif PLATFORM_LINUX
		return 0 == chdir(strPathOfNewCurrentDirectory.c_str());
#endif	
	}

	size_t getCPULogicalCoresCount(void)
	{
		return (size_t)std::thread::hardware_concurrency();
	}

	void getMemoryInfo(double& dGBSystemTotal, double& dGBSystemAvailable, double& dGBSystemUsed, double& dGBUsedByProcess)
	{
#ifdef PLATFORM_WINDOWS
		MEMORYSTATUSEX memStatus;
		memStatus.dwLength = sizeof(memStatus);
		GlobalMemoryStatusEx(&memStatus);
		dGBSystemTotal = memStatus.ullTotalPhys / (1024.0 * 1024.0 * 1024.0);
		dGBSystemAvailable = memStatus.ullAvailPhys / (1024.0 * 1024.0 * 1024.0);
		dGBSystemUsed = dGBSystemTotal - dGBSystemAvailable;
		PROCESS_MEMORY_COUNTERS pmc;
		GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
		dGBUsedByProcess = pmc.WorkingSetSize / (1024.0 * 1024.0 * 1024.0);
#elif PLATFORM_LINUX
		dGBSystemAvailable = 0.0;
		dGBSystemTotal = 0.0;
		dGBSystemUsed = 0.0;
#endif
	}

	// Class is only used on Windows and is used by getCPUUsage()
#ifdef PLATFORM_WINDOWS
	/// \brief Provides CPU usage information for each logical core with time-based averaging.
	class CPUUsage
	{
	public:
		/// \brief Constructor. Initializes the PDH query and counters.
		CPUUsage();

		/// \brief Destructor. Closes the PDH query.
		~CPUUsage();

		/// \brief Updates the CPU usage history with a new sample.
		void update();

		/// \brief Retrieves the averaged CPU usage for each core.
		/// \return Vector containing the averaged CPU usage percentages for each core.
		std::vector<float> getUsage() const;

		/// \brief Retrieves the CPU usage for the current process.
		/// \return CPU usage percentage of the current process.
		float getProcessUsage() const;

	private:
		PDH_HQUERY m_hQuery = NULL;                             ///< PDH query handle
		std::vector<PDH_HCOUNTER> m_vecCoreCounters;            ///< PDH counters for each core
		PDH_HCOUNTER m_hProcessCounter = NULL;                  ///< PDH counter for the current process
		DWORD m_dwNumCores = 0;                                 ///< Number of logical cores
		std::chrono::steady_clock::time_point m_lastUpdateTime; ///< Last data collection time
		bool m_bFirstSample = true;                             ///< Flag for first sample
	};

	// Helper function to get the instance name of the current process
	std::wstring GetProcessInstanceName()
	{
		DWORD processId = GetCurrentProcessId();
		wchar_t processName[MAX_PATH] = L"";
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
		if (hProcess)
		{
			HMODULE hMod;
			DWORD cbNeeded;
			if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
			{
				GetModuleBaseNameW(hProcess, hMod, processName, sizeof(processName) / sizeof(wchar_t));
			}
			CloseHandle(hProcess);
		}

		std::wstring processBaseName = processName;

		// PDH uses instance names with index if multiple processes have the same name
		// We'll enumerate the processes and match the IDs

		PDH_STATUS pdhStatus;
		PDH_HQUERY hQuery = NULL;
		PDH_HCOUNTER hCounter;
		DWORD dwBufferSize = 0;
		DWORD dwItemCount = 0;

		std::wstring counterPath = L"\\Process(*)\\ID Process";

		pdhStatus = PdhOpenQuery(NULL, NULL, &hQuery);
		if (pdhStatus != ERROR_SUCCESS)
		{
			return L"";
		}

		pdhStatus = PdhAddCounterW(hQuery, counterPath.c_str(), 0, &hCounter);
		if (pdhStatus != ERROR_SUCCESS)
		{
			PdhCloseQuery(hQuery);
			return L"";
		}

		pdhStatus = PdhCollectQueryData(hQuery);
		if (pdhStatus != ERROR_SUCCESS)
		{
			PdhCloseQuery(hQuery);
			return L"";
		}

		// First, get the required buffer size
		pdhStatus = PdhGetRawCounterArrayW(hCounter, &dwBufferSize, &dwItemCount, NULL);
		if (pdhStatus != PDH_MORE_DATA)
		{
			PdhCloseQuery(hQuery);
			return L"";
		}

		// Allocate buffer based on the required size
		std::vector<BYTE> buffer(dwBufferSize);
		PPDH_RAW_COUNTER_ITEM_W pItems = reinterpret_cast<PPDH_RAW_COUNTER_ITEM_W>(buffer.data());

		// Retrieve the actual data
		pdhStatus = PdhGetRawCounterArrayW(hCounter, &dwBufferSize, &dwItemCount, pItems);
		if (pdhStatus != ERROR_SUCCESS)
		{
			PdhCloseQuery(hQuery);
			return L"";
		}

		std::wstring instanceName;

		for (DWORD i = 0; i < dwItemCount; ++i)
		{
			if (pItems[i].RawValue.FirstValue == processId)
			{
				instanceName = pItems[i].szName;
				break;
			}
		}

		PdhCloseQuery(hQuery);
		return instanceName;
	}

	// Constructor
	CPUUsage::CPUUsage()
	{
		// Open a PDH query
		PDH_STATUS status = PdhOpenQuery(NULL, NULL, &m_hQuery);
		if (status != ERROR_SUCCESS)
		{
			//X::logError("Failed to open PDH query. Error code: %ld", status);
			return;
		}

		// Get the number of logical processors
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		m_dwNumCores = sysInfo.dwNumberOfProcessors;

		// Add a counter for each logical processor
		for (DWORD i = 0; i < m_dwNumCores; ++i)
		{
			std::wstring counterPath = L"\\Processor(" + std::to_wstring(i) + L")\\% Processor Time";
			PDH_HCOUNTER hCounter;
			status = PdhAddCounterW(m_hQuery, counterPath.c_str(), 0, &hCounter);
			if (status != ERROR_SUCCESS)
			{
				//X::logError("Failed to add PDH counter for core %lu. Error code: %ld", i, status);
				continue;
			}
			m_vecCoreCounters.push_back(hCounter);
		}

		// Add a counter for the current process CPU usage
		std::wstring instanceName = GetProcessInstanceName();
		if (instanceName.empty())
		{
			//X::logError("Failed to find PDH instance for current process.");
			m_hProcessCounter = NULL;
		}
		else
		{
			std::wstring counterPath = L"\\Process(" + instanceName + L")\\% Processor Time";
			status = PdhAddCounterW(m_hQuery, counterPath.c_str(), 0, &m_hProcessCounter);
			if (status != ERROR_SUCCESS)
			{
				//X::logError("Failed to add PDH counter for process. Error code: %ld", status);
				m_hProcessCounter = NULL;
			}
		}

		// Collect initial data
		PdhCollectQueryData(m_hQuery);
		m_lastUpdateTime = std::chrono::steady_clock::now();
		m_bFirstSample = true;
	}

	// Destructor
	CPUUsage::~CPUUsage()
	{
		PdhCloseQuery(m_hQuery);
	}

	// Update method
	void CPUUsage::update()
	{
		auto now = std::chrono::steady_clock::now();
		auto duration = now - m_lastUpdateTime;
		// Collect data only if at least 1 second has passed
		if (duration < std::chrono::seconds(1))
		{
			return;
		}

		// Collect new data
		PDH_STATUS status = PdhCollectQueryData(m_hQuery);
		if (status != ERROR_SUCCESS)
		{
			//X::logError("Failed to collect PDH query data. Error code: %ld", status);
			return;
		}

		m_lastUpdateTime = now;
		m_bFirstSample = false;
	}

	// Get CPU usage per core
	std::vector<float> CPUUsage::getUsage() const
	{
		std::vector<float> vecUsage;
		if (m_bFirstSample)
		{
			vecUsage.resize(m_dwNumCores, 0.0);
			return vecUsage;
		}

		for (size_t i = 0; i < m_vecCoreCounters.size(); ++i)
		{
			PDH_FMT_COUNTERVALUE counterVal;
			PDH_STATUS status = PdhGetFormattedCounterValue(
				m_vecCoreCounters[i], PDH_FMT_DOUBLE, NULL, &counterVal);
			if (status == ERROR_SUCCESS && counterVal.CStatus == ERROR_SUCCESS)
			{
				vecUsage.push_back((float)counterVal.doubleValue);
			}
			else
			{
				vecUsage.push_back(0.0f);
			}
		}
		return vecUsage;
	}

	// Get process CPU usage
	float CPUUsage::getProcessUsage() const
	{
		if (m_bFirstSample || m_hProcessCounter == NULL)
		{
			return 0.0;
		}

		PDH_FMT_COUNTERVALUE counterVal;
		PDH_STATUS status = PdhGetFormattedCounterValue(
			m_hProcessCounter, PDH_FMT_DOUBLE, NULL, &counterVal);
		if (status == ERROR_SUCCESS && counterVal.CStatus == ERROR_SUCCESS)
		{
			// Normalize the counter value by dividing by the number of logical processors
			float usage = float(counterVal.doubleValue / static_cast<double>(m_dwNumCores));

			// Clamp the usage to 0-100%
			if (usage < 0.0f) usage = 0.0f;
			if (usage > 100.0f) usage = 100.0f;

			return usage;
		}
		else
		{
			return 0.0f;
		}
	}
#endif

	std::vector<float> getCPUUsage(float& fSystemTotalCPUUsage, float& fProcessTotalCPUUsage)
	{
		std::vector<float> vecCPUUsage;
#ifdef PLATFORM_WINDOWS
		static CPUUsage CPUUsage;

		CPUUsage.update();
		
		vecCPUUsage = CPUUsage.getUsage();
		
		fSystemTotalCPUUsage = 0.0f;
		for (auto iCore : vecCPUUsage)
			fSystemTotalCPUUsage += iCore;
		fProcessTotalCPUUsage = CPUUsage.getProcessUsage();
#elif PLATFORM_LINUX
		fSystemTotalCPUUsage = 0.0f;
		fProcessTotalCPUUsage = 0.0f;
#endif
		return vecCPUUsage;
	}

/*

	glm::quat rotationBetweenVectors(glm::vec3 v1, glm::vec3 v2)
	{
		v1 = normalize(v1);
		v2 = normalize(v2);

		float cosTheta = dot(v1, v2);
		glm::vec3 rotationAxis;

		// Special case when both vectors are in opposite directions
		// There is no "ideal" rotation axis, so guess one. Any will do as long as it's perpendicular to v1
		if (cosTheta < -1 + 0.001f)
		{	
			rotationAxis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), v1);
			if (glm::length2(rotationAxis) < 0.01) // They were parallel
				rotationAxis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), v1);
			rotationAxis = normalize(rotationAxis);
			return glm::angleAxis(glm::radians(180.0f), rotationAxis);
		}
		rotationAxis = cross(v1, v2);
		float s = sqrt((1 + cosTheta) * 2);
		float invs = 1 / s;
		return glm::quat(s * 0.5f, rotationAxis.x * invs, rotationAxis.y * invs, rotationAxis.z * invs);
	}
*/
}