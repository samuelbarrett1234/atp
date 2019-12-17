#pragma once


// Author: Samuel Barrett


namespace atpsearch
{


template<typename T>
class XLock
{
public:
	typedef typename T Type;

	T& get()
	{
		return *m_pVal;
	}

private:
	T* m_pVal;
};


template<typename T>
struct SLock
{
public:
	typedef typename T Type;

	const T& get() const
	{
		return *m_pVal;
	}

private:
	const T* m_pVal;
};


} // namespace atpsearch


