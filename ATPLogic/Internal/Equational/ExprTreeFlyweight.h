#pragma once


/**

\file

\author Samuel Barrett

\brief Contains a helper class for efficient storage of data about
	Expression objects.
*/


#include <memory>
#include <vector>
#include <array>
#include "../../ATPLogicAPI.h"
#include "SyntaxNodes.h"


namespace atp
{
namespace logic
{
namespace equational
{


/**
\warning we impose a function arity limit for efficiency!
*/
static const constexpr size_t MAX_ARITY = 5;


/**
\brief This class encapsulates the complexity of storing an
	expression's syntax tree efficiently.

\details This is "efficient" because: the usage of `vector` makes the
	cache behaviour quite nice, and the "flyweight" properties mean
	that copies are only made when modifications need to be made, and
	we never copy unnecessarily. This class is supposed to have cheap
    copy-by-value operations.
*/
class ATP_LOGIC_API ExprTreeFlyweight
{
private:
    /**
    \brief Calling this on a shared pointer will create a branch of
        it for our usage as per copy-on-write semantics.

    \details If `ptr` has more than one owner then we create our own
        copy of the object, however if ptr has <= 1 reference counts
        then we do nothing.
    */
    template<typename T>
    void copy_on_write_branch(std::shared_ptr<T>& ptr)
    {
        // if there are multiple owners then reset our pointer to be to
        // a new value
        if (ptr.use_count() > 1)
        {
            ptr = std::make_shared<T>(*ptr);
        }
    }

public:
    /**
    \brief Create a new object from a binary stream, which must be of
        the correct format.

    \param in The (binary) input stream.
    */
    static ExprTreeFlyweight load_from_bin(std::istream& in);

public:
    ExprTreeFlyweight();
    ExprTreeFlyweight(const ExprTreeFlyweight& other);
    ExprTreeFlyweight(ExprTreeFlyweight&& other) noexcept;
    ExprTreeFlyweight& operator= (const ExprTreeFlyweight& other);
    ExprTreeFlyweight& operator= (ExprTreeFlyweight&& other) noexcept;

    inline size_t root_id() const
    {
        return m_root;
    }
    inline SyntaxNodeType root_type() const
    {
        return m_root_type;
    }
    void set_root(size_t id, SyntaxNodeType type);

    inline size_t size() const
    {
#ifdef ATP_LOGIC_DEFENSIVE
        _check_invariant();
#endif
        return m_func_symb_ids->size();
    }

    inline size_t func_symb_id(size_t idx) const
    {
        ATP_LOGIC_PRECOND(idx < m_func_symb_ids->size());
#ifdef ATP_LOGIC_DEFENSIVE
        _check_invariant();
#endif
        return m_func_symb_ids->at(idx);
    }
    inline size_t func_arity(size_t idx) const
    {
        ATP_LOGIC_PRECOND(idx < m_func_symb_ids->size());
#ifdef ATP_LOGIC_DEFENSIVE
        _check_invariant();
#endif
        return m_func_arity->at(idx);
    }
    inline const std::array<size_t, MAX_ARITY>&
        func_children(size_t idx) const
    {
        ATP_LOGIC_PRECOND(idx < m_func_symb_ids->size());
#ifdef ATP_LOGIC_DEFENSIVE
        _check_invariant();
#endif
        return m_func_children->at(idx);
    }
    inline const std::array<SyntaxNodeType, MAX_ARITY>&
        func_child_types(size_t idx) const
    {
        ATP_LOGIC_PRECOND(idx < m_func_symb_ids->size());
#ifdef ATP_LOGIC_DEFENSIVE
        _check_invariant();
#endif
        return m_func_child_types->at(idx);
    }

    /**
    \brief Add a new function to the internal arrays

    \returns The index of the newly created function (you will need
        this to reference it later)
    */
    template<typename ChildIterT,
        typename ChildTypeIterT>
    size_t add_func(size_t symb_id, size_t arity,
        ChildIterT child_begin, ChildIterT child_end,
        ChildTypeIterT child_type_begin,
        ChildTypeIterT child_type_end)
    {
#ifdef ATP_LOGIC_DEFENSIVE
        _check_invariant();
#endif

        ATP_LOGIC_PRECOND(arity <= MAX_ARITY);
        ATP_LOGIC_PRECOND(arity > 0);
        ATP_LOGIC_PRECOND(std::distance(child_begin,
            child_end) == arity);
        ATP_LOGIC_PRECOND(std::distance(child_type_begin,
            child_type_end) == arity);

        copy_on_write_branch(m_func_symb_ids);
        copy_on_write_branch(m_func_arity);
        copy_on_write_branch(m_func_children);
        copy_on_write_branch(m_func_child_types);

        m_func_arity->push_back(arity);
        m_func_symb_ids->push_back(symb_id);

        m_func_children->emplace_back();
        m_func_child_types->emplace_back();

        std::copy(child_begin, child_end,
            m_func_children->back().begin());

        std::copy(child_type_begin, child_type_end,
            m_func_child_types->back().begin());

        return m_func_children->size() - 1;
    }
        
    /**
    \brief Modify an existing function in the internal arrays at
        a given index.
    */
    template<typename ChildIterT,
        typename ChildTypeIterT>
    void update_func(size_t index, size_t symb_id, size_t arity,
        ChildIterT child_begin, ChildIterT child_end,
        ChildTypeIterT child_type_begin,
        ChildTypeIterT child_type_end)
    {
#ifdef ATP_LOGIC_DEFENSIVE
        _check_invariant();
#endif

        ATP_LOGIC_PRECOND(index < m_func_symb_ids->size());
        ATP_LOGIC_PRECOND(arity <= MAX_ARITY);
        ATP_LOGIC_PRECOND(arity > 0);
        ATP_LOGIC_PRECOND(std::distance(child_begin,
            child_end) == arity);
        ATP_LOGIC_PRECOND(std::distance(child_type_begin,
            child_type_end) == arity);

        copy_on_write_branch(m_func_symb_ids);
        copy_on_write_branch(m_func_arity);
        copy_on_write_branch(m_func_children);
        copy_on_write_branch(m_func_child_types);

        m_func_arity->at(index) = arity;
        m_func_symb_ids->at(index) = symb_id;

        std::copy(child_begin, child_end,
            m_func_children->at(index).begin());

        std::copy(child_type_begin, child_type_end,
            m_func_child_types->at(index).begin());
    }

    /**
    \brief Update a function's argument value and type

    \warning If `new_type == FUNC` then please ensure you have
        already merged the function data associated with this
        function into this object via `merge_from`. This is
        because, otherwise, this object would be left in a bad
        state after this call, as some of its indices would be
        invalid.
    */
    void update_func_child(size_t func_index, size_t arg_index,
        size_t new_id, SyntaxNodeType new_type)
    {
#ifdef ATP_LOGIC_DEFENSIVE
        _check_invariant();
#endif

        ATP_LOGIC_PRECOND(func_index < m_func_arity->size());
        ATP_LOGIC_PRECOND(arg_index < m_func_arity->at(func_index));
        ATP_LOGIC_PRECOND(new_type != SyntaxNodeType::EQ);

        // check that if we are setting it to a function, then the
        // function index is valid:
        ATP_LOGIC_PRECOND(new_type != SyntaxNodeType::FUNC ||
            new_id < m_func_arity->size());

        copy_on_write_branch(m_func_children);
        copy_on_write_branch(m_func_child_types);

        m_func_children->at(func_index).at(arg_index) = new_id;
        m_func_child_types->at(func_index).at(arg_index) = new_type;
    }

    /**
    \brief Append, to each array in this object, the arrays in the
        other object. Also, update their function indices to make
        them still consistent.

    \details This is a key function of this object, and hiding the
        complexity of this operation was the key reason to factor
        out this logic into an external class.

    \note If `other` does not have root function type, then this
        operation does nothing.

    \warning This should be done **before** using `update_func_child`
        to re-point children already present in this node (if any).

    \returns The ID of the root of `other`, adjusted if necessary.
        When we say "adjusted", we mean as follows: if other does not
        have function root type, then we just return other.root().
        If it does have function root type, then we return the index
        of the root function **adjusted as a result of the merge**.
    */
    size_t merge_from(const ExprTreeFlyweight& other);

    /**
    \brief Save this object to the given (binary) output

    \param out The (binary) output stream.
    */
    void save(std::ostream& out) const;

#ifdef ATP_LOGIC_DEFENSIVE
private:
    void _check_invariant() const;
#endif

private:
    // the root ID/index and the type of the root node
    size_t m_root;
    SyntaxNodeType m_root_type;

    // m_func_symb_ids[i] is the symbol ID of the ith function node
	std::shared_ptr<std::vector<size_t>> m_func_symb_ids;

    // m_func_arity[i] is the arity of the ith function
    std::shared_ptr<std::vector<size_t>> m_func_arity;

    // m_func_children[i] is the array of size `m_func_arity[i]` of
    // children of that function, and m_func_child_types[i] the
    // corresponding types.
    // if m_func_child_types[i][j] == SyntaxNodeType::FREE,
    // then m_func_children[i][j] is the free variable ID.
    // if m_func_child_types[i][j] == SyntaxNodeType::CONSTANT,
    // then m_func_children[i][j] is the constant symbol ID.
    // if m_func_child_types[i][j] == SyntaxNodeType::FUNC,
    // then m_func_children[i][j] is the index of the function in
    // these arrays.

    std::shared_ptr<std::vector<std::array<size_t,
        MAX_ARITY>>> m_func_children;

    std::shared_ptr<std::vector<std::array<SyntaxNodeType,
        MAX_ARITY>>> m_func_child_types;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


