#include "pch.h"

#include "logger.h"
#include <iostream>

// initialization of static members
std::queue<std::string> eddft3::Logger::queue;
std::mutex eddft3::Logger::mutex;
std::condition_variable eddft3::Logger::cv;
std::thread eddft3::Logger::worker;
std::atomic<bool> eddft3::Logger::running{ false };
FILE* eddft3::Logger::consoleOut = nullptr;

void eddft3::Logger::initialize()
{
	if (running) return;

	AllocConsole();

	// disable close button to prevent ES closing
	HWND hwnd = GetConsoleWindow();
	if (hwnd != NULL)
	{
		HMENU hMenu = GetSystemMenu(hwnd, FALSE);
		if (hMenu != NULL)
		{
			DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
		}
		SetWindowTextA(hwnd, "eddft3 Logger");
	}

	freopen_s(&consoleOut, "CONOUT$", "w", stdout);

	// create thread
	running = true;
	worker = std::thread(workerThread);

	log("Logger initialized successfully.");
}

void eddft3::Logger::shutdown()
{
	if (!running) return;

	// signal thread stopping
	running = false;
	cv.notify_all();

	// wait on closing thread
	if (worker.joinable()) worker.join();

	if (consoleOut)
	{
		fclose(consoleOut);
		consoleOut = nullptr;
	}
	FreeConsole();
}

void eddft3::Logger::log(const std::string& message)
{
	if (!running) return;

	{
		std::lock_guard<std::mutex> lock(mutex);
		queue.push(message);
	}
	cv.notify_one();
}

void eddft3::Logger::workerThread()
{
	while (running)
	{
		std::string msg;

		// wait if queue is not empty or if worker is not running
		{
			std::unique_lock<std::mutex> lock(mutex);
			cv.wait(lock, [] { return !queue.empty() || !running; });

			if (!running && queue.empty())
				break;

			msg = queue.front();
			queue.pop();
		}

		// send the msg
		std::cout << msg << std::endl;
	}
}