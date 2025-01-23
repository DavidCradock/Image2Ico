#pragma once
/// \file Globals.h When all global variables are located so we can control their order of creation and destruction.

namespace X
{
	/// \brief Forward declaration of classes so we don't need to inclide the header files here.
	class CLog;
	class CProfiler;

	/// \brief Class to hold all global variables
	class CGlobals
	{
	public:

		/// \brief Constructor
		CGlobals();

		/// \brief Destructor
		~CGlobals();

		/// \brief Initialises all global variables
		void init(void);

		/// \brief Pointer to the logging object
		CLog* pLog;

		/// \brief Pointer to the profiler object
		CProfiler* pProfiler;
	};
	extern CGlobals* pGlobals;	///< Pointer to object of CGlobals class holding all the global variables
}