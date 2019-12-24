#include <memory>
#include <thread>
#include <vector>
#include <ProcessManager.h>
#include "TestProcess.h"
#include "BufferResource.h"


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
	g_pProcMgr = std::make_unique<ProcessManager>(
		ProcessSchedulerType::WORK_STEALING,
		ResourceOperationSchedulerType::WORK_STEALING,
		LockManagementType::WOUND_WAIT
	);

	const size_t proc_threads = 2U, io_threads = 2U;

	std::vector<std::thread> threads;

	for (size_t i = 0; i < proc_threads; i++)
	{
		threads.emplace_back(&proc_thread_func);
	}
	for (size_t i = 0; i < io_threads; i++)
	{
		threads.emplace_back(&io_thread_func);
	}

	//TODO: set up an initial batch of resources and processes

	//TODO: wait for a bit

	g_pProcMgr->stop();

	for (auto& t : threads)
		t.join();

	return 0;
}


