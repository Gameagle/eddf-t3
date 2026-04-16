#pragma once

#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

namespace eddft3
{
	class Logger
	{
	public:
		//************************************
		// Description: Initializes the logger by allocating a console and starting the worker thread
		// Method:    initialize
		// FullName:  eddft3::Logger::initialize
		// Access:    public static 
		// Returns:   void
		// Qualifier:
		//************************************
		static void initialize();
		
		//************************************
		// Description: Shuts down the logger by stopping the worker thread and freeing the console
		// Method:    shutdown
		// FullName:  eddft3::Logger::shutdown
		// Access:    public static 
		// Returns:   void
		// Qualifier:
		//************************************
		static void shutdown();
		
		//************************************
		// Description: Logs a message by pushing it into the queue for the worker thread to process
		// Method:    log
		// FullName:  eddft3::Logger::log
		// Access:    public static 
		// Returns:   void
		// Qualifier:
		// Parameter: const std::string & message
		//************************************
		static void log(const std::string& message);

		//************************************
		// Description: Returns if the logger is currently running
		// Method:    isRunning
		// FullName:  eddft3::Logger::isRunning
		// Access:    public static 
		// Returns:   bool
		// Qualifier: inline
		//************************************
		inline static bool isRunning() { return running; };

	private:
		//************************************
		// Description: Worker thread function that continuously processes log messages from the queue and outputs them to the console
		// Method:    workerThread
		// FullName:  eddft3::Logger::workerThread
		// Access:    private static 
		// Returns:   void
		// Qualifier:
		//************************************
		static void workerThread();

		static std::queue<std::string> queue;
		static std::mutex mutex;
		static std::condition_variable cv;
		static std::thread worker;
		static std::atomic<bool> running;
		static FILE* consoleOut;
	};
}