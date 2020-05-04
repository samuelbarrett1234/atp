#pragma once


/**
\file

\author Samuel Barrett

\brief Contains an implementation of a process which runs an
	automated conjecturing procedure.
*/


#include <sstream>
#include <ATPLogic.h>
#include <ATPDatabase.h>
#include "IProcess.h"


/**
\brief This is a process which will run an automated conjecturing
	procedure continuously.
*/
class ConjectureProcess :
	public IProcess
{
public:
	ConjectureProcess(atp::db::DatabasePtr p_db,
		atp::logic::LanguagePtr p_lang);

	bool done() const override;
	bool waiting() const override;
	void run_step() override;
	inline void dump_log(std::ostream& out) override
	{
		out << m_out.str();
		m_out = std::stringstream();
	}

private:
	atp::db::DatabasePtr m_db;
	atp::logic::LanguagePtr m_lang;
	std::stringstream m_out;

};


