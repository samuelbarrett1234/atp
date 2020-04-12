#pragma once


/**

\file

\author Samuel Barrett

\brief Implementation of the IKnowledgeKernel for equational logic

*/


#include <map>
#include <string>
#include <vector>
#include <list>
#include <boost/bimap.hpp>
#include "../../ATPLogicAPI.h"
#include "../../Interfaces/IKnowledgeKernel.h"
#include "Statement.h"
#include "StatementArray.h"
#include "ModelContext.h"


namespace atp
{
namespace logic
{
namespace equational
{


class ATP_LOGIC_API KnowledgeKernel :
	public IKnowledgeKernel
{
private:
	// use builder function instead of constructing this way
	KnowledgeKernel(const ModelContext& ctx);

public:
	static KnowledgeKernelPtr try_construct(const Language& lang,
		const ModelContext& ctx);

	size_t get_integrity_code() const override;

	ProofStatePtr begin_proof_of(
		const IStatement& stmt) const override;

	bool is_trivial(
		const IStatement& stmt) const override;

	inline const std::vector<Statement>& get_active_rules() const
	{
		return m_rules;
	}

private:
	/**
	\brief Checks that all symbol IDs and arities agree with those
		defined in the model context.

	\returns true iff success.
	*/
	static bool type_check(const ModelContext& ctx,
		const Statement& stmt);

private:
	const ModelContext& m_context;

	// all equality rules (given as axioms)
	// use vector not StatementArray because we need mutability
	// (we need to .push_back as we go):
	std::vector<Statement> m_rules;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


