/**
\file

\author Samuel Barrett

\brief This suite tests the helper classes in `Data.h`

*/


#include <Interfaces/Data.h>
#include <Internal/Equational/Statement.h>
#include "Test.h"
#include "LogicSetupFixture.h"


using namespace atp::db;
using atp::logic::StmtFormat;


BOOST_AUTO_TEST_SUITE(DataTests);


BOOST_AUTO_TEST_CASE(test_int_dvalue)
{
	DValue dv((int)7);

	BOOST_TEST(dv.type() == DType::INT);

	BOOST_TEST(dv.as_int() == 7);
}


BOOST_AUTO_TEST_CASE(test_uint_dvalue)
{
	DValue dv((size_t)144);

	BOOST_TEST(dv.type() == DType::UINT);

	BOOST_TEST(dv.as_uint() == 144);
}


BOOST_AUTO_TEST_CASE(test_float_dvalue)
{
	DValue dv((float)3.14f);

	BOOST_TEST(dv.type() == DType::FLOAT);

	BOOST_TEST(dv.as_float() == 3.14f);
}


BOOST_AUTO_TEST_CASE(test_bool_dvalue)
{
	DValue dv(true);

	BOOST_TEST(dv.type() == DType::BOOL);

	BOOST_TEST(dv.as_bool() == true);
}


BOOST_AUTO_TEST_CASE(test_str_dvalue)
{
	DValue dv((std::string)"test");

	BOOST_TEST(dv.type() == DType::STR);

	BOOST_TEST(dv.as_str() == "test");
}


BOOST_FIXTURE_TEST_CASE(test_stmt_dvalue,
	LogicSetupFixture)
{
	s << "i(i(x)) = x";

	auto p_stmts = p_lang->deserialise_stmts(
		s, StmtFormat::TEXT, *p_ctx);

	DValue dv(p_stmts->at(0));

	BOOST_TEST(dv.type() == DType::STMT);

	auto p_stmt = dynamic_cast<atp::logic::equational::Statement*
		>(dv.as_stmt().get());

	BOOST_TEST(p_stmt != nullptr);

	BOOST_TEST(p_stmt->equivalent(*dynamic_cast<
		const atp::logic::equational::Statement*>(&p_stmts->at(0))));
}


BOOST_AUTO_TEST_CASE(test_int_darray)
{
	DArray darr(std::vector<int>{ 1, 3, 4, 6 });

	BOOST_TEST(darr.type() == DType::INT);
	BOOST_TEST(darr.size() == 4);
	BOOST_TEST(!darr.empty());
	BOOST_TEST(darr.val_at(1).as_int() == 3);
}


BOOST_AUTO_TEST_CASE(test_uint_darray)
{
	DArray darr(std::vector<size_t>{ 1, 3, 4, 6 });

	BOOST_TEST(darr.type() == DType::UINT);
	BOOST_TEST(darr.size() == 4);
	BOOST_TEST(!darr.empty());
	BOOST_TEST(darr.val_at(1).as_uint() == 3);
}


BOOST_AUTO_TEST_CASE(test_float_darray)
{
	DArray darr(std::vector<float>{ 1.0f, 3.0f, 4.0f, 6.0f });

	BOOST_TEST(darr.type() == DType::FLOAT);
	BOOST_TEST(darr.size() == 4);
	BOOST_TEST(!darr.empty());
	BOOST_TEST(darr.val_at(1).as_float() == 3.0f);
}


BOOST_AUTO_TEST_CASE(test_bool_darray)
{
	DArray darr(std::vector<bool>{ true, false, false, true });

	BOOST_TEST(darr.type() == DType::FLOAT);
	BOOST_TEST(darr.size() == 4);
	BOOST_TEST(!darr.empty());
	BOOST_TEST(darr.val_at(1).as_bool() == false);
}


BOOST_AUTO_TEST_CASE(test_str_darray)
{
	DArray darr(std::vector<std::string>{
		"test1", "test2", "test3"
	});

	BOOST_TEST(darr.type() == DType::STR);
	BOOST_TEST(darr.size() == 3);
	BOOST_TEST(!darr.empty());
	BOOST_TEST(darr.val_at(1).as_str() == "test2");
}


BOOST_FIXTURE_TEST_CASE(test_stmt_darray,
	LogicSetupFixture)
{
	s << "i(i(x)) = x \n";
	s << "*(i(x), i(i(x))) = e";

	auto p_stmts = p_lang->deserialise_stmts(s, StmtFormat::TEXT,
		*p_ctx);

	DArray arr(std::move(p_stmts));

	BOOST_TEST(arr.type() == DType::STMT);
	BOOST_TEST(arr.size() == 2);
	BOOST_TEST(!arr.empty());
	BOOST_TEST(arr.val_at(0).as_stmt()->to_str() == "i(i(x0)) = x0");
}


BOOST_AUTO_TEST_CASE(test_int_darray_from_dvalues)
{
	DArray darr(std::vector<DValue>{ DValue((int)1),
		DValue((int)7) });

	BOOST_TEST(darr.type() == DType::INT);
	BOOST_TEST(darr.size() == 2);
	BOOST_TEST(!darr.empty());
	BOOST_TEST(darr.val_at(1).as_int() == 7);
}


BOOST_FIXTURE_TEST_CASE(test_stmt_darray_from_dvalues,
	LogicSetupFixture)
{
	s << "i(i(x)) = x \n";
	s << "*(i(x), i(i(x))) = e";

	auto p_stmts = p_lang->deserialise_stmts(s, StmtFormat::TEXT,
		*p_ctx);

	DArray arr(std::vector<DValue>{ DValue(p_stmts->at(0)),
		DValue(p_stmts->at(1)) });

	BOOST_TEST(arr.type() == DType::STMT);
	BOOST_TEST(arr.size() == 2);
	BOOST_TEST(!arr.empty());
	BOOST_TEST(arr.val_at(0).as_stmt()->to_str() == "i(i(x0)) = x0");
}


BOOST_AUTO_TEST_CASE(test_column_str)
{
	Column col("col name");

	BOOST_TEST(col.has_name());
	BOOST_TEST(!col.has_index());
	BOOST_TEST(col.name() == "col name");
}


BOOST_AUTO_TEST_CASE(test_column_idx_no_name_array)
{
	Column col(7);

	BOOST_TEST(col.has_index());
	BOOST_TEST(!col.has_name());
	BOOST_TEST(col.index() == 7);
}


BOOST_AUTO_TEST_CASE(test_column_idx_and_name_array)
{
	std::vector<std::string> col_names{
		"col name 1", "col name 2", "col name 3"
	};

	Column col(1, &col_names);

	BOOST_TEST(col.has_index());
	BOOST_TEST(col.has_name());
	BOOST_TEST(col.index() == 1);
	BOOST_TEST(col.name() == "col name 2");
}


BOOST_AUTO_TEST_CASE(test_column_list_from_str)
{
	ColumnList col_list(std::vector<std::string>{
		"col name 1", "col name 2", "col name 3"
	});

	BOOST_TEST(col_list.contains("col name 3"));
	BOOST_TEST(col_list.size() == 3);
	BOOST_TEST(!col_list.empty());
	BOOST_TEST(col_list.at(1).name() == "col name 2");
}


BOOST_AUTO_TEST_CASE(test_column_list_from_indices)
{
	std::vector<std::string> col_names{
		"col name 1", "col name 2", "col name 3"
	};

	// remark: we have reversed the order here, to make sure that the
	// column list is ordered as per the indices, not as per the
	// `col_names` array!

	ColumnList col_list(std::vector<size_t>{ 2, 1, 0 },
		&col_names);

	BOOST_TEST(col_list.contains("col name 3"));
	BOOST_TEST(col_list.size() == 3);
	BOOST_TEST(!col_list.empty());
	BOOST_TEST(col_list.at(2).name() == "col name 1");
}


BOOST_AUTO_TEST_CASE(insert_test)
{
	std::vector<std::string> col_names{
		"col name 1", "col name 2", "col name 3",
		"col name 4"
	};

	ColumnList col_list(std::vector<size_t>{ 0 },
		&col_names);

	// there are many different special cases involved in `insert`...
	// try all of them!

	col_list.insert(1);  // index 1
	BOOST_TEST(col_list.at(1).name() == "col name 2");

	col_list.insert(Column(2, &col_names));
	BOOST_TEST(col_list.at(2).name() == "col name 3");

	col_list.insert("col name 4");
	BOOST_TEST(col_list.at(3).name() == "col name 4");
}


BOOST_AUTO_TEST_CASE(col_list_insert_test_no_name_array)
{
	ColumnList col_list;

	BOOST_TEST(col_list.empty());

	col_list.insert("col name 1");
	BOOST_TEST(col_list.contains("col name 1"));
	BOOST_TEST(col_list.size() == 1);
	BOOST_TEST(!col_list.empty());
	BOOST_TEST(col_list.at(0).name() == "col name 1");

	std::vector<std::string> some_names{
		"col name 2", "col name 3"
	};

	// this is subtle: col_list has no parent name array, but the col
	// we are inserting DOES - check that `col_list` knows to convert
	// the column to a name string and then insert it.
	col_list.insert(Column(0, &some_names));

	BOOST_TEST(col_list.size() == 2);
	BOOST_TEST(col_list.at(1).name() == "col name 2");
}


BOOST_AUTO_TEST_CASE(test_index_of_for_value_version_of_col_list)
{
	ColumnList col_list({
		"col name 1", "col name 2", "col name 3",
		"col name 4" });


	// there are many different ways in which we can get the index
	// of a Column object - test all of them!

	// find by name
	BOOST_TEST(col_list.index_of("col name 4") == 3);

	// find by name, but going via a separate list
	std::vector<std::string> col_names{
		"col name 1", "col name 2", "col name 3",
		"col name 4"
	};
	BOOST_TEST(col_list.index_of(Column(1, &col_names)) == 1);

	// find by index, which is just the identity function
	BOOST_TEST(col_list.index_of(Column(2)) == 2);
}


BOOST_AUTO_TEST_CASE(test_index_of_for_pointer_version_of_col_list)
{
	std::vector<std::string> col_names{
		"col name 1", "col name 2", "col name 3",
		"col name 4"
	};

	ColumnList col_list(std::vector<size_t>{ 3, 2, 1, 0 },
		&col_names);

	// there are many different ways in which we can get the index
	// of a Column object - test all of them!

	// find by name
	BOOST_TEST(col_list.index_of("col name 1") == 3);

	// find by name, but going via a potentially separate list, and
	// in this case - a list in a different order
	BOOST_TEST(col_list.index_of(Column(1, &col_names)) == 2);

	// find by index, which is just the identity function
	BOOST_TEST(col_list.index_of(Column(2)) == 2);
}


BOOST_AUTO_TEST_SUITE_END();


