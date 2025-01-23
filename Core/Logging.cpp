#include "Logging.h"
#include "Exceptions.h"
#include "StringUtils.h"
#include "Utilities.h"
#include <filesystem>
// For writing out to console, but only in debug builds as the release builds do not have a console (See project settings/C++/Preprocessor Definitions _CONSOLE for debug and _WINDOWS for release)
#ifdef _DEBUG
#include <iostream>		// For std::cout << std::string;
#endif

namespace X
{
	CLogEntry::CLogEntry()
	{
		fTimeSeconds = 0.0f;
		iTimeMin = 0;
		iTimeHours = 0;
		iTimeDays = 0;
		iTimeWeeks = 0;
	}

	CLog::CLog(const std::string& strFilename)
	{
		// Whether to log each type of entry with the various LOG macros
		mbLogNormalEntries = true;
		mbLogEmptyLines = true;
		mbLogHeadings = true;
		mbLogVerboseEntries = true;
		mbLogErrorEntries = true;
		mbLogSingleLineEntries = true;

		_mstrFilename = strFilename;
		_mStop = false;

		// Set private html code used when writing each column of each row in the table to the html file.
		// The contents of each cell in the table is written to the file after each one of these strings.
		_strTableColumnText[0] = "<tr><td width=\"1%\"><div align=\"left\">";
		_strTableColumnText[1] = "</div></td><td width=\"58%\"><div align=\"left\">";
		_strTableColumnText[2] = "</div></td><td width=\"20%\"><div align=\"left\">";
		_strTableColumnText[3] = "</div></td><td width=\"1%\"><div align=\"left\">";
		_strTableColumnText[4] = "</div></td><td width=\"20%\"><div align=\"left\">";

		_writeLogHeader();

		_mMainThread = std::thread(&CLog::_mainThreadLoop, this);
	}

	CLog::~CLog()
	{
		_mStop = true;
		_mMainThread.join();

		_writeLogFooter();
	}

	void CLog::add(const std::string& strText, const std::string& strFunctionName, const std::string& strLineNumber, const std::string& strSourceFilename, bool bStripPathFromSourceFilename, const CColourf& textColour)
	{
		_mQueueMutex.lock();

		CLogEntry entry;
		entry.eType = CLogEntry::ENTRY_TYPE_NORMAL;
		entry.strText = strText;
		entry.strFunctionName = strFunctionName;
		if (bStripPathFromSourceFilename)
			entry.strSourceFilename = StringUtils::getFilenameFromFullPath(strSourceFilename);
		else
			entry.strSourceFilename = strSourceFilename;
		entry.strSourceLineNumber = strLineNumber;
		entry.textColour = textColour;

		// Compute time related stuff
		_timer.update();
		_timer.getClock(entry.fTimeSeconds, entry.iTimeMin, entry.iTimeHours, entry.iTimeDays, entry.iTimeWeeks);

		// Create the current runtime as a string in the log entry
		if (entry.iTimeWeeks > 0)
			entry.strTime += std::to_string(entry.iTimeWeeks) + "w:";
		if (entry.iTimeDays > 0)
			entry.strTime += std::to_string(entry.iTimeDays) + "d:";
		if (entry.iTimeHours > 0)
			entry.strTime += std::to_string(entry.iTimeHours) + "h:";
		if (entry.iTimeMin > 0)
			entry.strTime += std::to_string(entry.iTimeMin) + "m:";
		if (entry.fTimeSeconds < 10.0f)
			entry.strTime += "0";

		// Set seconds to 3 decimal places
		std::ostringstream oss;
		oss.precision(2); // Set the number of decimal places
		oss << std::fixed << entry.fTimeSeconds;
		entry.strTime += oss.str() + "s";
		//entry.strTime += std::to_string(float(entry.fTimeSeconds)) + "s";

		// Add entry to the queue
		_mQueueEntriesToAdd.push(entry);

		// Add entry to logging window
		//gShared._mSharedLogging.addEntry(entry.strText, entry.strFunctionName, entry.strSourceLineNumber, entry.strSourceFilename, entry.strTime, false, CColourf(1, 1, 1, 1));

		// Write out to console, but only in debug builds as the release builds do not have a console (See project settings/C++/Preprocessor Definitions _CONSOLE for debug and _WINDOWS for release)
		#ifdef _DEBUG
		std::string strConsoleText = entry.strTime + " " + entry.strText + " " + entry.strFunctionName + " " + entry.strSourceLineNumber + " " + entry.strSourceFilename + "\n";
		std::cout << strConsoleText;
		#endif
		_mQueueMutex.unlock();
	}

	void CLog::addEmptyLine(void)
	{
		_mQueueMutex.lock();
		CLogEntry entry;
		entry.eType = CLogEntry::ENTRY_TYPE_EMPTY_LINE;
		_mQueueEntriesToAdd.push(entry);

		// Add entry to logging window
		//gShared._mSharedLogging.addEntry("", "", "", "", "", false, CColourf(1, 1, 1, 1));

		// Write out to console, but only in debug builds as the release builds do not have a console (See project settings/C++/Preprocessor Definitions _CONSOLE for debug and _WINDOWS for release)
		#ifdef _DEBUG
		std::string strConsoleText = entry.strTime + " " + entry.strText + " " + entry.strFunctionName + " " + entry.strSourceLineNumber + " " + entry.strSourceFilename + "\n";
		std::cout << "\n";
		#endif
		_mQueueMutex.unlock();
	}

	void CLog::addHeading(const std::string& strHeading)
	{
		_mQueueMutex.lock();
		CLogEntry entry;
		entry.eType = CLogEntry::ENTRY_TYPE_HEADING;
		entry.strText = strHeading;
		_mQueueEntriesToAdd.push(entry);

		//gShared._mSharedLogging.addEntry(entry.strText, "", "", "", "", true, CColourf(1, 1, 1, 1));

		// Write out to console, but only in debug builds as the release builds do not have a console (See project settings/C++/Preprocessor Definitions _CONSOLE for debug and _WINDOWS for release)
		#ifdef _DEBUG
		std::string strConsoleText = entry.strTime + " " + entry.strText + " " + entry.strFunctionName + " " + entry.strSourceLineNumber + " " + entry.strSourceFilename + "\n";
		std::cout << entry.strText + "\n";
		#endif
		_mQueueMutex.unlock();
	}

	void CLog::addSingleLine(const std::string& strText, const CColourf& textColour)
	{
		_mQueueMutex.lock();
		CLogEntry entry;
		entry.eType = CLogEntry::ENTRY_TYPE_SINGLE_LINE;
		entry.strText = strText;
		entry.textColour = textColour;
		_mQueueEntriesToAdd.push(entry);

		//gShared._mSharedLogging.addEntry(entry.strText, "", "", "", "", true, CColourf(1, 1, 1, 1));

		// Write out to console, but only in debug builds as the release builds do not have a console (See project settings/C++/Preprocessor Definitions _CONSOLE for debug and _WINDOWS for release)
#ifdef _DEBUG
		std::string strConsoleText = entry.strText + "\n";
		std::cout << entry.strText + "\n";
#endif
		_mQueueMutex.unlock();
	}

	void CLog::_mainThreadLoop(void)
	{
		bool bLogToSave = false;
		CLogEntry logToSave;

		while (!_mStop)	// This gets set to true in the destructor
		{
			// Determine whether there's at least one log entry in the queue
			// If so, copy it ready to save to file.
			// We perform the minimal of stuff here to help prevent locking the mutex which could slow down calls to add()
			bLogToSave = false;
			_mQueueMutex.lock();
			if (!_mQueueEntriesToAdd.empty())
			{
				bLogToSave = true;
				logToSave = _mQueueEntriesToAdd.front();
				_mQueueEntriesToAdd.pop();
			}
			_mQueueMutex.unlock();

			// If there is a log entry, save it to file
			if (bLogToSave)
			{
				std::ofstream file(_mstrFilename, std::ios::app);
				if (file.is_open())
				{
					if (CLogEntry::ENTRY_TYPE_EMPTY_LINE == logToSave.eType)
					{
						// We close the table, write out a single line and then re-open the table
						// End table
						file << "</table>\n";

						// Write out single line
						file << "<br>";

						// Re-open table
						file << "<table width=\"100%\" border=\"0\">\n";
					}
					else if (CLogEntry::ENTRY_TYPE_HEADING == logToSave.eType)
					{
						file << _strTableColumnText[0];
						file << _strTableColumnText[1] << "<b>" << logToSave.strText << "</b>";
						file << _strTableColumnText[2];
						file << _strTableColumnText[3];
						file << _strTableColumnText[4];
						file << "</div></td></tr>\n";	// End table row
					}
					else if (CLogEntry::ENTRY_TYPE_NORMAL == logToSave.eType)
					{
						file << _strTableColumnText[0] << logToSave.strTime;
						//file << _strTableColumnText[1] << logToSave.strText;
						file << _strTableColumnText[1] << "<span style=\"color:" << logToSave.textColour.colourToHexStringRGB() << "\">" << logToSave.strText << "</span>";
						file << _strTableColumnText[2] << logToSave.strFunctionName;
						file << _strTableColumnText[3] << logToSave.strSourceLineNumber;
						file << _strTableColumnText[4] << logToSave.strSourceFilename;
						file << "</div></td></tr>\n";	// End table row
					}
					else if (CLogEntry::ENTRY_TYPE_SINGLE_LINE == logToSave.eType)
					{
						// We close the table, write out a single line and then re-open the table
						// End table
						file << "</table>\n";

						// Write out single line
						file << "<span style=\"color:" << logToSave.textColour.colourToHexStringRGB() << "\">" << logToSave.strText << "</span>";

						// Re-open table
						file << "<table width=\"100%\" border=\"0\">\n";
					}
				}
				else
				{
				}
			}
			else
			{
				// If there wasn't a log entry to save, let's sleep for a bit.
				// Not sleeping due to there being a log entry makes it so that on the next
				// loop is fast and allows us to prevent entries from queueing up.
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		}

		// Now that we're stopping, write out any logs.
		// Add one last entry...
		LOG("Log file closing. Writing out the last of the added entries...");
		while (!_mQueueEntriesToAdd.empty())
		{
			logToSave = _mQueueEntriesToAdd.front();
			_mQueueEntriesToAdd.pop();

			std::ofstream file(_mstrFilename, std::ios::app);
			if (file.is_open())
			{
				file << _strTableColumnText[0] << logToSave.strTime;
				file << _strTableColumnText[1] << logToSave.strText;
				file << _strTableColumnText[2] << logToSave.strFunctionName;
				file << _strTableColumnText[3] << logToSave.strSourceLineNumber;
				file << _strTableColumnText[4] << logToSave.strSourceFilename;
				file << "</div></td></tr>\n";	// End table row
			}
		}

	}

	void CLog::run_test(void)
	{
		CLog logger("log_run_test.txt");
		std::thread t1([&logger]()
			{
				for (int i = 0; i < 100; ++i) {
					logger.add("Thread 1: Message " + std::to_string(i), __FUNCTION__, std::to_string(__LINE__), __FILE__);
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
				}
			});

		std::thread t2([&logger]() {
			for (int i = 0; i < 100; ++i) {
				logger.add("Thread 2: Message " + std::to_string(i), __FUNCTION__, std::to_string(__LINE__), __FILE__);
				std::this_thread::sleep_for(std::chrono::milliseconds(15));
			}
			});

		t1.join();
		t2.join();
	}

	void CLog::_writeLogHeader(void)
	{
		// Clear file and add header
		std::ofstream file(_mstrFilename, std::ios::trunc);
		// Write styles and start body
		file << "<html>\n<head>\n<style>\n";
		file << "body {	background-color: rgba(0, 0, 0, 1.0); color: rgba(255, 255, 255, 1.0); }\n";
		file << "tr:nth-child(even) { background-color: rgba(10, 10, 10, 0.8); color: rgba(250, 250, 250, 1.0); }\n";
		file << "tr:nth-child(odd) { background-color: rgba(20, 20, 20, 0.8); color: rgba(250, 250, 250, 1.0); }\n";
		file << "</style>\n</head>\n<body>";
		
		// Write heading
		file << "<h1 style = \"text-align:center\">" << _mstrFilename << "</h1>\n";

		// Write begin table and write first row with headings for each column
		file << "<table width=\"100%\" border=\"0\">\n";
		file << _strTableColumnText[0] << "Time";
		file << _strTableColumnText[1] << "Description";
		file << _strTableColumnText[2] << "Namespace:Class:Method";
		file << _strTableColumnText[3] << "Line #";
		file << _strTableColumnText[4] << "Source Filename";
		file << "</div></td></tr>\n";	// End table row
	}

	void CLog::_writeLogFooter(void)
	{
		std::ofstream file(_mstrFilename, std::ios::app);
		// End table, body and html tags
		file << "</table>\n</body>\n</html>\n";
		file.close();
	}
}