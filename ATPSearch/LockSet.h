#pragma once


// Author: Samuel Barrett


#include "Utility.h"
#include "Error.h"
#include "Lock.h"
#include "ProcessStatus.h"
#include <string>
#include <type_traits>


namespace atpsearch
{


/// <summary>
/// A lock set is a collection of locks of various different
/// resources which can be acquired separately at various
/// stages in a process's execution. It handles the construction
/// of a lock request process status, and also tracks which have
/// been acquired and which haven't.
/// This version of the class is for singleton sets of locks.
/// </summary>
template<typename T>
class LockSet
{
public:
    typedef typename T Type;

public:
    /// <summary>
    /// Get the resource associated with the ith lock.
    /// Precondition: the lock must have already been
    /// acquired, and since this verison of LockSet is
    /// for singleton lists, i==0.
    /// </summary>
    template<size_t i>
    typename T::Type& get()
    {
        static_assert(i == 0);
        ATP_CHECK_PRE(acquired<i>());

        return m_lock.get();
    }

    /// <summary>
    /// Check if ith lock has been acquired.
    /// Precondition: since this verison of LockSet is
    /// for singleton lists, i==0.
    /// </summary>
    template<size_t i>
    bool acquired()
    {
        static_assert(i == 0);

        return m_bAcquired;
    }

    /// <summary>
    /// Acquire the ith lock.
    /// Precondition: since this verison of LockSet is
    /// for singleton lists, i==0. Furthermore, the ith
    /// lock must not have been acquired previously.
    /// </summary>
    /// <param name="id">Some identifier with the resource,
    /// e.g. a filename. This ID is resource-type-specific.
    /// </param>
    template<size_t i>
    ProcessStatus acquire(std::string id)
    {
        static_assert(i == 0);
        ATP_CHECK_PRE(!acquired<i>());

        ProcessStatus proc_stat;
        proc_stat.action = ProcessStatus::Action::LOCK_REQUEST;
        proc_stat.lock_request.id = id;
        proc_stat.lock_request.val_ptr = &m_lock.pVal;
        return proc_stat;
    }

private:
    T m_lock;
    bool m_bAcquired = false;
};


/// <summary>
/// A lock set is a collection of locks of various different
/// resources which can be acquired separately at various
/// stages in a process's execution. It handles the construction
/// of a lock request process status, and also tracks which have
/// been acquired and which haven't.
/// This version of the class is for non-singleton sets of locks.
/// This class works by splitting the list of locks into a head
/// (the first element) and the tail (the rest of the elements)
/// and utilises the singleton LockSet class for each element in the list.
/// </summary>
template<typename T, typename... Ts>
class LockSet
{
public:
    typedef typename T Type;
private:
    template<size_t, typename>
    struct elem_type_holder;

    template<typename X, typename... Xs>
    struct elem_type_holder<0, LockSet<X, Xs...>>
    {
        typedef X Type;
    };

    template<size_t i, typename X, typename... Xs>
    struct elem_type_holder<i, LockSet<X, Xs...>>
    {
        typedef typename elem_type_holder<i - 1, LockSet<Xs...>>::Type Type;
    };

public:

    /// <summary>
    /// Get the resource associated with the ith lock.
    /// Precondition: the ith lock must have already been
    /// acquired.
    /// </summary>
    template<size_t i>
    typename elem_type_holder<i, LockSet<T, Ts...>>::Type::Type& get()
    {
        if (i == 0)
        {
            return m_headLock.get<0>();
        }
        else
        {
            return m_tailLock.get<i - 1>();
        }
    }


    /// <summary>
    /// Check if ith lock has been acquired.
    /// </summary>
    template<size_t i>
    bool acquired()
    {
        if (i == 0)
        {
            return m_headLock.acquired<0>();
        }
        else
        {
            return m_tailLock.acquired<i - 1>();
        }
    }


    /// <summary>
    /// Acquire the ith lock.
    /// Precondition: the ith lock must not have been
    /// acquired previously.
    /// </summary>
    /// <param name="id">Some identifier with the resource,
    /// e.g. a filename. This ID is resource-type-specific.
    /// </param>
    template<size_t i>
    ProcessStatus acquire(std::string id)
    {
        if (i == 0)
        {
            return m_headLock.acquire<0>(id);
        }
        else
        {
            return m_tailLock.acquire<i - 1>(id);
        }
    }

private:
    // split lock list into head element & tail list
    LockSet<T> m_headLock;
    LockSet<Ts...> m_tailLock;
};


} // namespace atpsearch


