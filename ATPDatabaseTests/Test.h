#pragma once


#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <Interfaces/Data.h>


namespace std
{
static inline ostream& boost_test_print_type(ostream& os,
	atp::db::DType dt)
{
	switch (dt)
	{
	case atp::db::DType::INT:
		os << "int";
		break;
	case atp::db::DType::UINT:
		os << "uint";
		break;
	case atp::db::DType::FLOAT:
		os << "float";
		break;
	case atp::db::DType::STR:
		os << "str";
		break;
	case atp::db::DType::STMT:
		os << "stmt";
		break;
	}
	return os;
}
}  // namespace std


