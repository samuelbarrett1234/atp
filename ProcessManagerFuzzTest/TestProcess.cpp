#include "TestProcess.h"


TestProcess::TestProcess(size_t num_resources, ProcessManager& proc_mgr) :
	m_NumResources(num_resources),
	m_ProcMgr(proc_mgr)
{
}


ProcessStatus TestProcess::tick()
{
	ProcessStatus result;


	return result;
}


void TestProcess::abort()
{
}


