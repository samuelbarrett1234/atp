#pragma once


#include <Process.h>
#include <ProcessManager.h>


using atpsearch::IProcess;
using atpsearch::ProcessManager;
using atpsearch::ProcessStatus;


class TestProcess :
	public IProcess
{
public:
	TestProcess(size_t num_resources,
		ProcessManager& proc_mgr);

	ProcessStatus tick() override;

	void abort() override;

	inline std::string get_name() const override
	{
		return "";
	}

	inline std::string get_details() const override
	{
		return "";
	}

private:
	const size_t m_NumResources;  // the total number of resources
	ProcessManager& m_ProcMgr;
};


