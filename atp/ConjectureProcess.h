#pragma once


/**
\file

\author Samuel Barrett

\brief Contains an implementation of a process which runs an
	automated conjecturing procedure.
*/


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
		atp::logic::LanguagePtr p_lang, bool training_mode);

	bool done() const override;
	bool waiting() const override;
	void run_step() override;

private:
	atp::db::DatabasePtr m_db;
	atp::logic::LanguagePtr m_lang;

};


