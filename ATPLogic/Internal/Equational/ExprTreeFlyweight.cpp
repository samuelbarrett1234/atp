/**

\file

\author Samuel Barrett

*/


#include "ExprTreeFlyweight.h"


namespace atp
{
namespace logic
{
namespace equational
{


ExprTreeFlyweight::ExprTreeFlyweight() :
	// using this constructor is a little dangerous because we have
	// no reasonable default values for m_root and m_root_type,
	// hence give it a VERY BAD value and hope the user overrides it
	// before using the object!
	m_root_type(SyntaxNodeType::EQ)
{ }


ExprTreeFlyweight::ExprTreeFlyweight(
	const ExprTreeFlyweight& other) :
	m_root(other.m_root), m_root_type(other.m_root_type),
	m_func_symb_ids(other.m_func_symb_ids),
	m_func_arity(other.m_func_arity),
	m_func_children(other.m_func_children),
	m_func_child_types(other.m_func_child_types)
{
#ifdef ATP_LOGIC_DEFENSIVE
	_check_invariant();
#endif
}


ExprTreeFlyweight::ExprTreeFlyweight(
	ExprTreeFlyweight&& other) noexcept :
	m_root(other.m_root), m_root_type(other.m_root_type),
	m_func_symb_ids(std::move(other.m_func_symb_ids)),
	m_func_arity(std::move(other.m_func_arity)),
	m_func_children(std::move(other.m_func_children)),
	m_func_child_types(std::move(other.m_func_child_types))
{
	// give it a BAD TYPE! To indicate that the object is in an
	// invalid state
	other.m_root_type = SyntaxNodeType::EQ;

#ifdef ATP_LOGIC_DEFENSIVE
	_check_invariant();
#endif
}


ExprTreeFlyweight& ExprTreeFlyweight::operator=(
	const ExprTreeFlyweight& other)
{
	if (this != &other)
	{
		m_root = other.m_root;
		m_root_type = other.m_root_type;
		m_func_symb_ids = other.m_func_symb_ids;
		m_func_arity = other.m_func_arity;
		m_func_children = other.m_func_children;
		m_func_child_types = other.m_func_child_types;

#ifdef ATP_LOGIC_DEFENSIVE
		_check_invariant();
#endif
	}
	return *this;
}


ExprTreeFlyweight& ExprTreeFlyweight::operator=(
	ExprTreeFlyweight&& other) noexcept
{
	if (this != &other)
	{
		m_root = other.m_root;
		m_root_type = other.m_root_type;
		m_func_symb_ids = std::move(other.m_func_symb_ids);
		m_func_arity = std::move(other.m_func_arity);
		m_func_children = std::move(other.m_func_children);
		m_func_child_types = std::move(other.m_func_child_types);

		// give it a BAD TYPE! To indicate that the object is in an
		// invalid state
		other.m_root_type = SyntaxNodeType::EQ;

#ifdef ATP_LOGIC_DEFENSIVE
		_check_invariant();
#endif
	}
	return *this;
}


void ExprTreeFlyweight::set_root(size_t id, SyntaxNodeType type)
{
	// don't check invariant beforehand, because the object starts
	// in an invalid state when default-constructed

	ATP_LOGIC_PRECOND(type != SyntaxNodeType::FUNC ||
		id < m_func_symb_ids.size());

	m_root = id;
	m_root_type = type;

	if (type != SyntaxNodeType::FUNC)
	{
		// don't need any of this if we are not a function node type
		m_func_symb_ids.clear();
		m_func_arity.clear();
		m_func_children.clear();
		m_func_child_types.clear();
	}

#ifdef ATP_LOGIC_DEFENSIVE
	_check_invariant();
#endif
}


size_t ExprTreeFlyweight::merge_from(const ExprTreeFlyweight& other)
{
#ifdef ATP_LOGIC_DEFENSIVE
	_check_invariant();
	other._check_invariant();
#endif

	switch (other.m_root_type)
	{
	case SyntaxNodeType::FREE:
	case SyntaxNodeType::CONSTANT:
		return other.root_id();  // nothing to do
	case SyntaxNodeType::FUNC:
		m_func_symb_ids.insert(m_func_symb_ids.end(),
			other.m_func_symb_ids.begin(),
			other.m_func_symb_ids.end());

		m_func_arity.insert(m_func_arity.end(),
			other.m_func_arity.begin(),
			other.m_func_arity.end());

		m_func_children.insert(m_func_children.end(),
			other.m_func_children.begin(),
			other.m_func_children.end());

		m_func_child_types.insert(m_func_child_types.end(),
			other.m_func_child_types.begin(),
			other.m_func_child_types.end());

		// offset the root index by the increment:
		return other.m_root + other.m_func_symb_ids.size();
	default:
		ATP_LOGIC_ASSERT(false && "invalid syntax node type!");
		throw std::exception();
	}
}


#ifdef ATP_LOGIC_DEFENSIVE
void ExprTreeFlyweight::_check_invariant() const
{
	if (m_root_type == SyntaxNodeType::FUNC)
	{
		ATP_LOGIC_ASSERT(!m_func_symb_ids.empty());
		ATP_LOGIC_ASSERT(m_func_symb_ids.size() ==
			m_func_arity.size());
		ATP_LOGIC_ASSERT(m_func_symb_ids.size() ==
			m_func_children.size());
		ATP_LOGIC_ASSERT(m_func_symb_ids.size() ==
			m_func_child_types.size());
		ATP_LOGIC_ASSERT(m_root < m_func_symb_ids.size());
	}
}
#endif


}  // namespace equational
}  // namespace logic
}  // namespace atp


