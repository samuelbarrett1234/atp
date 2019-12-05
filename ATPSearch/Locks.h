#pragma once



// Author: Samuel Barrett


#include <typeinfo>


namespace atp
{


class XLockGeneric
{
public:

private:
	void* m_pObject;
	size_t m_ObjHashCode;

};


class SLockGeneric
{
public:
};


template<typename Val_t>
class XLock
{
public:
	XLock(XLockGeneric& internalLock) :
		m_lock(internalLock)
	{ }

private:
	XLockGeneric& m_lock;
};


template<typename Val_t>
class SLock
{
public:
	SLock(SLockGeneric& internalLock) :
		m_lock(internalLock)
	{ }

private:
	SLockGeneric& m_lock;
};


} // namespace atp


