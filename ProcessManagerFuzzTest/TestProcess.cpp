#include "TestProcess.h"
#include <Error.h>


static std::random_device g_static_eng;


TestProcess::TestProcess(size_t num_resources, ProcessManager& proc_mgr) :
	m_NumResources(num_resources),
	m_ProcMgr(proc_mgr),
	m_NumTicksRequired(std::uniform_int_distribution<size_t>(1U, 1000U)(g_static_eng)),
	m_MainBuffer(std::uniform_int_distribution<size_t>(0U, num_resources - 1U)(g_static_eng)),
	m_SomeData(std::uniform_real_distribution<float>(0.0f, 1.0f)(g_static_eng)),
	m_bDidResOpLastTick(false)
{
}


ProcessStatus TestProcess::tick()
{
	ProcessStatus result;

	ATP_CHECK_INVARIANT(m_NumTicksRequired > 0,
		"A process should not be ticked() after it has finished!");

	m_NumTicksRequired--;

	if (m_bDidResOpLastTick)
	{
		for (size_t i = 0; i < 4; i++)
		{
			ATP_CHECK_POSTCOND(m_SizeParam[i] == sizeof(m_SomeData),
				"Checking buffer read/write amounts from resop.");
		}

		ATP_CHECK_POSTCOND(m_SomeData == m_Placeholder,
			"Values should've been read/written correctly.");
	}
	else
	{
		// Leave enough ticks in order to check results:
		if (m_NumTicksRequired == 0)
			m_NumTicksRequired++;

		// Construct some resource operations:

		const size_t targetBuffer = std::uniform_int_distribution<size_t>(0U, m_NumResources - 1U)(g_static_eng);

		result.ops.push_back(atpsearch::resop::write(m_MainBuffer, &m_SomeData, sizeof(m_SomeData), m_SizeParam + 0));
		result.ops.push_back(atpsearch::resop::pipe(targetBuffer, m_MainBuffer, sizeof(m_SomeData), m_SizeParam + 1, 1U));
		result.ops.push_back(atpsearch::resop::pipe(m_MainBuffer, targetBuffer, sizeof(m_SomeData), m_SizeParam + 2, 2U));
		result.ops.push_back(atpsearch::resop::read(m_MainBuffer, &m_Placeholder, sizeof(m_Placeholder), m_SizeParam + 3, 3U));

		// Finally, reset this so that the procmgr can't "get away with" not doing anything:
		m_Placeholder = -1.0f;
	}

	result.bFinished = (m_NumTicksRequired == 0);

	// TODO: Post some new operations?

	return result;
}


void TestProcess::abort()
{
	m_NumTicksRequired = std::uniform_int_distribution<size_t>(1U, 1000U)(g_static_eng);
}


