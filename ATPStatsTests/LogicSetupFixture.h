#pragma once


#include <sstream>
#include <ATPLogic.h>
#include "DefinitionStrs.h"


struct LogicSetupFixture
{
	LogicSetupFixture()
	{
		// use group theory context
		std::stringstream defn_in(
			group_theory_definition_str);

		p_lang = atp::logic::create_language(
			atp::logic::LangType::EQUATIONAL_LOGIC);
		p_ctx = p_lang->try_create_context(defn_in);
		p_ker = p_lang->try_create_kernel(*p_ctx);
	}

	std::stringstream s;
	atp::logic::LanguagePtr p_lang;
	atp::logic::ModelContextPtr p_ctx;
	atp::logic::KnowledgeKernelPtr p_ker;
};


