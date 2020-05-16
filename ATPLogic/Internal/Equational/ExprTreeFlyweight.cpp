/**

\file

\author Samuel Barrett

*/


#include "ExprTreeFlyweight.h"
#include <boost/serialization/serialization.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/array.hpp>


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
	m_root_type(SyntaxNodeType::EQ),
	// create empty arrays
	m_func_info(std::make_shared<std::pair<std::vector<size_t>,
		std::vector<size_t>>>()),
	m_func_children(std::make_shared<std::pair<std::vector<std::array<
		size_t, MAX_ARITY>>, std::vector<std::array<
		SyntaxNodeType, MAX_ARITY>>>>())
{ }


ExprTreeFlyweight::ExprTreeFlyweight(
	const ExprTreeFlyweight& other) :
	m_root(other.m_root), m_root_type(other.m_root_type),
	m_func_info(other.m_func_info),
	m_func_children(other.m_func_children)
{
#ifdef ATP_LOGIC_DEFENSIVE
	_check_invariant();
#endif
}


ExprTreeFlyweight::ExprTreeFlyweight(
	ExprTreeFlyweight&& other) noexcept :
	m_root(other.m_root), m_root_type(other.m_root_type),
	m_func_info(std::move(other.m_func_info)),
	m_func_children(std::move(other.m_func_children))
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
		m_func_info = other.m_func_info;
		m_func_children = other.m_func_children;

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
		m_func_info = std::move(other.m_func_info);
		m_func_children = std::move(other.m_func_children);

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
		id < m_func_info->first.size());

	m_root = id;
	m_root_type = type;

	if (type != SyntaxNodeType::FUNC)
	{
		// don't need any of this if we are not a function node type
		m_func_info->first.clear();
		m_func_info->second.clear();
		m_func_children->first.clear();
		m_func_children->second.clear();
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
	{
		const size_t size_before = size();

		copy_on_write_branch(m_func_info);
		copy_on_write_branch(m_func_children);

		m_func_info->first.insert(m_func_info->first.end(),
			other.m_func_info->first.begin(),
			other.m_func_info->first.end());

		m_func_info->second.insert(m_func_info->second.end(),
			other.m_func_info->second.begin(),
			other.m_func_info->second.end());

		m_func_children->first.insert(m_func_children->first.end(),
			other.m_func_children->first.begin(),
			other.m_func_children->first.end());

		m_func_children->second.insert(m_func_children->second.end(),
			other.m_func_children->second.begin(),
			other.m_func_children->second.end());

		// now we need to alter all function indices we just added
		// by offsetting them by `size_before`, and finally we do
		// the same thing to the root (that we return)
		
		for (size_t i = size_before; i < m_func_children->first.size();
			++i)
		{
			for (size_t j = 0; j < m_func_info->second.at(i); ++j)
			{
				if (m_func_children->second.at(i).at(j) ==
					SyntaxNodeType::FUNC)
				{
					m_func_children->first.at(i).at(j) += size_before;
				}
			}
		}

		// offset the root index by the increment:
		return other.m_root + size_before;
	}
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
		ATP_LOGIC_ASSERT(m_func_info != nullptr);
		ATP_LOGIC_ASSERT(m_func_children != nullptr);

		ATP_LOGIC_ASSERT(!m_func_info->first.empty());
		ATP_LOGIC_ASSERT(m_func_info->first.size() ==
			m_func_info->second.size());
		ATP_LOGIC_ASSERT(m_func_info->first.size() ==
			m_func_children->first.size());
		ATP_LOGIC_ASSERT(m_func_info->first.size() ==
			m_func_children->second.size());
		ATP_LOGIC_ASSERT(m_root < m_func_info->first.size());
	}
}
#endif


ExprTreeFlyweight ExprTreeFlyweight::load_from_bin(std::istream& in)
{
	ExprTreeFlyweight result;

	auto i = boost::archive::binary_iarchive(in);

	// this must appear in the same order as it does in `save`

	i >> result.m_root >> result.m_root_type;
	i >> result.m_func_info->first >> result.m_func_info->second;
	i >> result.m_func_children->first >> result.m_func_children->second;

#ifdef ATP_LOGIC_DEFENSIVE
	// check for corruption

	ATP_LOGIC_PRECOND((result.m_root_type == SyntaxNodeType::FREE)
		|| (result.m_root_type == SyntaxNodeType::CONSTANT)
		|| (result.m_root_type == SyntaxNodeType::FUNC));

	result._check_invariant();
#endif

	return result;
}


void ExprTreeFlyweight::save(std::ostream& out) const
{
	auto o = boost::archive::binary_oarchive(out);

	// this must appear in the same order as it does in
	// `load_from_bin`

	o << m_root << m_root_type << m_func_info->first;
	o << m_func_info->second << m_func_children->first;
	o << m_func_children->second;
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


