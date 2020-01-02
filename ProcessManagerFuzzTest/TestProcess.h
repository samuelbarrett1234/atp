#pragma once


// Author: Samuel Barrett


#include <Process.h>
#include <ProcessManager.h>
#include <random>


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
	size_t m_NumTicksRequired;  // the number of ticks() needed until this process is done
	size_t m_SizeParam[4];  // how much of the buffer has been read/written (filled in by proc.mgr. after a res-op.)
	size_t m_MainBuffer;  // the resource ID of the buffer used for this process
	const float m_SomeData;  // the initial value of the above
	float m_Placeholder;  // a location to put data
	bool m_bDidResOpLastTick;  // true iff we did a resource operation last tick() and need to check its result on the next tick()
};


