#pragma once
#include "../Globals.h"
#include "Utilities.h"
#include "Timer.h"
#include "DataStructures/Colourf.h"
//#include "../Applications/Shared/Shared.h"	// For logging to the logging window by accessing the gShared._mSharedLogging.addEntry() method
#include <fstream>
#include <string>
#include <vector>
#include <mutex>
#include <queue>
#include <thread>


namespace X
{
	/// \brief A Log entry object which holds information about a single log entry. Used by the CLog class
	/// 
	/// See CLog class for more information.
	class CLogEntry
	{
	public:
		CLogEntry();

		enum EEntryType
		{
			ENTRY_TYPE_NORMAL,		///< Normal 5 column log entry
			ENTRY_TYPE_HEADING,		///< Heading
			ENTRY_TYPE_EMPTY_LINE,	///< Empty line
			ENTRY_TYPE_SINGLE_LINE	///< Closes the table, writes out a single line and then re-opens the table
		};
		std::string strText;			///< A string holding the text for this log entry.
		std::string strFunctionName;	///< A string holding the function name. Obtained from preprocessor predefined macro __FUNCTION__
		std::string strSourceFilename;	///< A string holding the source code filename. Obtained from preprocessor predefined macro __FILE__
		std::string strSourceLineNumber;///< A string holding the source code line number. Obtained from preprocessor predefined macro std::to_string(__LINE__)
		CColourf textColour;			///< The colour of the text to add to the log file
		float fTimeSeconds;				///< Time since start of log file creation that log entry was added (Seconds as clock)
		int iTimeMin;					///< Time since start of log file creation that log entry was added (Minutes as clock)
		int iTimeHours;					///< Time since start of log file creation that log entry was added (Hours as clock)
		int iTimeDays;					///< Time since start of log file creation that log entry was added (Days as clock)
		int iTimeWeeks;					///< Time since start of log file creation that log entry was added (Weeks as clock)
		std::string strTime;			///< Time since start of log file creation that log entry was added, formatted to a string
		EEntryType eType;				///< The type of entry this is. Normal, heading or empty line etc.
	};

	/// \brief Logging of text to a text file.
	///
	/// Logging of information to a text file
	/// The default filename used for the log file is "log.html" but can be changed with the constructor of this class, passing the filename we wish to use
	/// Example...
	/// \code
	/// CLog myLog("myLog.html");		// Create a logging object using the given filename.
	/// 
	/// // Add an entry to the log file...
	/// myLog.add("Some text");	// Text to be added to the log entry in the file.
	/// \endcode
	/// 
	/// There are several macros in this header file, more information about each one is given below, they are...
	/// LOG, LOGHEADING, LOGEMPTYLINE, LOGVERBOSE, LOGERROR and LOGSINGLELINE.
	/// 
	/// LOG, LOGVERBOSE and LOGERROR each accept a string which should hold some descriptive text for the log entry.
	/// These macros adds not only the text, but also the function the macro is called from as well as the line number and the source code file's name.
	/// Depending upon the macro used, the text colour is set to a different colour.
	/// For LOG, the colour is white
	/// For LOGVERBOSE, the colour is grey
	/// For LOGERROR, the colour is red.
	/// 
	/// LOGHEADING and LOGEMPTYLINE are used for formatting purposes.
	/// 
	/// There is also a macro LOGSINGLELINE which accepts a string and a colour. This is used to log text with a specific colour.
	/// It is used by the Vulkan debug callback when logging Vulkans debug messages.
	/// The log file typically has 5 columns, from left to right, they are time stamp, the log entry's text, Namespace::Class::Method the macro was called from, the line number and the source code file's name.
	/// However, with LOGSINGLELINE, the table is closed, then the text is written out with the given colour and then the table is re-opened.
	/// 
	/// The CLog class has several boolean members which can be set to true or false to enable or disable logging of the various log entry types. By default, they are all set to true.
	/// These are mbLogNormalEntries, mbLogHeadings, mbLogEmptyLines, mbLogVerboseEntries, mbLogErrorEntries and mbLogSingleLineEntries.
	/// 
	/// Ideas/suggestions for use of the various log entry types...
	///	LOG for routine operations and checkpoints. I suggest NOT adding the classes name to the entries text, IE LOG("CSomeClass::Method() called."); as these are already added and make the text addition redundant.
	///	LOGHEADING to mark the start and end of major phases like initialization and shutdown.
	/// LOGEMPTYLINE for separating sections of the log file.
	///	LOGVERBOSE for detailed debugging information, especially during development and troubleshooting.
	///	LOGERROR for critical errors that require immediate attention and may affect the application's operation. This is called when an exception is thrown.
	/// 
	/// Throughout X, these LOG macros are called instead of calling the gLogMain object's add() method as it reduces the amount of stuff we need to type.
	/// When an exception is thrown, the LOGERROR macro is called to log the exception to the file.
	/// When add() is called, regardless of either directly or from the macro, the actual log object is written with threading in mind, so that the call to add() has minimal overhead.
	/// The object has a main thread loop which periodically checks to see if add() has been called and writes those log entries to a file in the seperate thread.
	class CLog
	{
	public:
		/// \brief Constructor
		///
		/// Upon construction, the passed filename is deleted.
		CLog(const std::string& strFilename = "log.html");

		/// \brief Destructor
		~CLog();

		/// \brief Add text to the log file
		///
		/// \param strText String holding the text for this new log entry.
		/// \param strFunctionName String holding the function name. Obtained from preprocessor predefined macro __FUNCTION__
		/// \param strLineNumber String holding the source code line number. Obtained from preprocessor predefined macro std::to_string(__LINE__)
		/// \param strSourceFilename String holding the source code filename. Obtained from preprocessor predefined macro __FILE__
		/// \param bStripPathFromSourceFilename If true, the path is stripped from the source filename
		/// \param textColour The colour of the text to add to the log file
		/// 
		/// Instead of having to manually call something like the following, which can get quite tedious...
		/// \code
		/// clogObject.add("DescriptiveText", __FUNCTION__, std::to_string(__LINE__), __FILE__);
		/// \endcode
		/// We have a macro which accepts just the strText variable, so we can succinctly type...
		/// \code
		/// LOG("DescriptiveText");
		/// \endcode
		/// Much easier :) Most of X uses this macro.
		void add(const std::string& strText, const std::string& strFunctionName, const std::string& strLineNumber, const std::string& strSourceFilename, bool bStripPathFromSourceFilename = false, const CColourf& textColour = CColourf(1, 1, 1, 1));

		/// \brief Adds an empty line with no time, text or anything. Simply for formatting the log file.
		void addEmptyLine(void);

		/// \brief Adds a heading to the log file
		/// 
		/// \param strHeading The text to add as a heading
		void addHeading(const std::string& strHeading);

		/// \brief Adds a log entry with a specific colour, as one single line with just one column instead of the usual five.
		///
		/// This closes the 5 column table, writes out the strText with the given colour and then re-opens the table.
		/// It's used by the Vulkan validation layer debug callback.
		void addSingleLine(const std::string& strText, const CColourf& textColour = CColourf(1,1,1,1));

		/// \brief Test method to test logging
		///
		/// Writes some text to the filename log_run_test.txt
		void run_test(void);

		bool mbLogNormalEntries;		///< Whether to log normal entries with the macro LOG
		bool mbLogHeadings;				///< Whether to log headings with the macro LOGHEADING
		bool mbLogEmptyLines;			///< Whether to log empty lines with the macro LOGEMPTYLINE
		bool mbLogVerboseEntries;		///< Whether to log verbose entries with the macro LOGVERBOSE
		bool mbLogErrorEntries;			///< Whether to log error entries with the macro LOGERROR
		bool mbLogSingleLineEntries;	///< Whether to log entries with a specific colour withthe macro LOGSINGLELINE
	private:
		std::string _mstrFilename;					///< Holds the filename of the log file passed to the constructor
		std::queue<CLogEntry> _mQueueEntriesToAdd;	///< Queue of CLogEntry objects which have been added with a call to add() which the main logging thread checks to write to a file
		std::mutex _mQueueMutex;					///< Mutex to allow access to the _mQueueEntriesToAdd in a thread safe way
		std::thread _mMainThread;					///< Main logging thread which checks the _mQueueEntriesToAdd and writes to a file and pops them from the queue.
		bool _mStop;								///< Whether to stop the main thread this class uses.
		CTimer _timer;								///< For adding time to the beginning of each entry

		/// \brief Main loop which pops log entries from the queue and writes them to a file.
		void _mainThreadLoop(void);

		/// \brief Writes header to the log file
		///
		/// Also opens the log file with the std::ios::trunc flag so that the file is cleared
		void _writeLogHeader(void);

		/// \brief Writes out the end of the html file, closing body and html tags etc
		void _writeLogFooter(void);

		/// \brief Each element of this array holds the html text to add before adding the table cell's contents (text)
		///
		/// For example, element 0 holds the following html... 
		/// <tr><td width=\"1%\"><div align=\"center\">" and then the text would be added to the file after this to be contained within the first column of the table
		/// We have the html code stored in these so that when they are added from various methods, the width settings remain consistant and we only have to change them
		/// where these strings are setup (In the constructor of CLog)
		std::string _strTableColumnText[5];
	};

	/// \brief Macro to call gLogMain.add() passing in __FUNCTION__, __FILE__ and __LINE__ The colour of the text will be white.
	///
	/// Usage:
	/// \code
	/// LOG("Some log info text");
	/// // Will add "Some log info text 
	/// \endcode
#ifndef	LOG
#define LOG(x) {																								\
		if (pGlobals->pLog->mbLogNormalEntries)																	\
			{pGlobals->pLog->add(x, __FUNCTION__, std::to_string(__LINE__), __FILE__, true, CColourf(1,1,1,1));}	\
	}
#endif

/// \brief Macro to call gLogMain.addHeading()
///
/// Usage:
/// \code
/// LOGHEADING("Some heading text");
/// \endcode
#ifndef	LOGHEADING
#define LOGHEADING(x) {							\
		if (pGlobals->pLog->mbLogHeadings)		\
			{pGlobals->pLog->addHeading(x);}	\
	}
#endif

/// \brief Macro to call gLogMain.addEmptyLine()
///
/// Usage:
/// \code
/// LOGEMPTYLINE("Some log info text");
/// \endcode
#ifndef	LOGEMPTYLINE
#define LOGEMPTYLINE {							\
		if (pGlobals->pLog->mbLogEmptyLines)	\
			{pGlobals->pLog->addEmptyLine();}	\
	}
#endif

/// \brief Macro to call gLogMain.add() passing in __FUNCTION__, __FILE__ and __LINE__ The colour of the text will be grey to signify a verbose log entry.
///
/// Usage:
/// \code
/// LOGVERBOSE("Some log info text");
/// // Will add "Some log info text 
/// \endcode
#ifndef	LOGVERBOSE
#define LOGVERBOSE(x) {																										\
		if (pGlobals->pLog->mbLogVerboseEntries)																			\
			{pGlobals->pLog->add(x, __FUNCTION__, std::to_string(__LINE__), __FILE__, true, CColourf(0.6f, 0.6f, 0.6f, 1));}	\
	}
#endif

/// \brief Macro to call gLogMain.add() passing in __FUNCTION__, __FILE__ and __LINE__ The colour of the text will be yellow to signify an error log entry.
///
/// Usage:
/// \code
/// LOGERROR("Some log info text");
/// // Will add "Some log info text 
/// \endcode
#ifndef	LOGERROR
#define LOGERROR(x) {																							\
		if (pGlobals->pLog->mbLogErrorEntries)																	\
			{pGlobals->pLog->add(x, __FUNCTION__, std::to_string(__LINE__), __FILE__, true, CColourf(1,0,0,1));}	\
	}
#endif

/// \brief Macro to call gLogMain.addSingleLine()
///
/// Usage:
/// \code
/// LOGSINGLELINE("Some log info text", CColourf(1,1,1,1));
/// // Will add "Some log info text " with the specified colour. This calls addSingleLine()
/// \endcode
#ifndef	LOGSINGLELINE
#define LOGSINGLELINE(x, c) {																							\
		if (pGlobals->pLog->mbLogSingleLineEntries)																	\
			{pGlobals->pLog->addSingleLine(x, c);}	\
	}
#endif



}