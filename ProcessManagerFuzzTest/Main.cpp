#include <memory>
#include <thread>
#include <vector>
#include <ProcessManager.h>
#include <fstream>
#include "TestProcess.h"
#include "BufferResource.h"


// Author: Samuel Barrett


using atpsearch::ProcessManager;
using atpsearch::ProcessSchedulerType;
using atpsearch::ResourceOperationSchedulerType;
using atpsearch::LockManagementType;


std::unique_ptr<ProcessManager> g_pProcMgr;


void proc_thread_func()
{
	g_pProcMgr->run_processes();
}


void io_thread_func()
{
	g_pProcMgr->run_io();
}


int main(int argc, char* argv[])
{
	try
	{
		g_pProcMgr = std::make_unique<ProcessManager>(
			ProcessSchedulerType::WORK_STEALING,
			ResourceOperationSchedulerType::WORK_STEALING,
			LockManagementType::WOUND_WAIT
			);

		// Parameters:

		const size_t proc_threads = 2U, io_threads = 2U;
		const size_t num_resources = 8U, initial_num_processes = 512U;
		const size_t running_seconds = 4U;

		// Set up worker threads

		std::vector<std::thread> threads;

		for (size_t i = 0; i < proc_threads; i++)
		{
			threads.emplace_back(&proc_thread_func);
		}
		for (size_t i = 0; i < io_threads; i++)
		{
			threads.emplace_back(&io_thread_func);
		}

		// Set up an initial batch of resources and processes

		for (size_t i = 0; i < num_resources; i++)
		{
			g_pProcMgr->register_resource(std::make_unique<BufferResource>(i));
		}

		for (size_t i = 0; i < initial_num_processes; i++)
		{
			g_pProcMgr->post(std::make_unique<TestProcess>(num_resources, *g_pProcMgr));
		}

		// Wait for a bit

		std::this_thread::sleep_for(std::chrono::seconds(running_seconds));

		// Halt the program

		g_pProcMgr->stop();

		for (auto& t : threads)
			t.join();

		return 0;
	}
	catch (std::exception& ex)
	{
		std::ofstream output("fuzz_test_output_" PROJECT_CONFIGPLATFORM ".txt", std::ios::ate);
		output << ex.what() << std::endl;
		output.close();
	}
}


