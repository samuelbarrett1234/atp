#pragma once


#include <sstream>
#include <Internal/Equational/Language.h>
#include <Internal/Equational/ModelContext.h>
#include <Internal/Equational/KnowledgeKernel.h>


struct EmptyTestFixture
{
	std::stringstream s,
		ctx_in;
	atp::logic::equational::Language lang;
	atp::logic::ModelContextPtr p_ctx;
	const atp::logic::equational::ModelContext& ctx;
	atp::logic::KnowledgeKernelPtr p_ker;
	atp::logic::equational::KnowledgeKernel& ker;

	EmptyTestFixture() :
		ctx_in("{}"),  // empty file
		p_ctx(lang.try_create_context(ctx_in)),
		ctx(dynamic_cast<const
			atp::logic::equational::ModelContext&>(*p_ctx)),
		p_ker(lang.try_create_kernel(ctx)),
		ker(dynamic_cast<
			atp::logic::equational::KnowledgeKernel&>(*p_ker))
	{
		s << std::noskipws;
	}
};


