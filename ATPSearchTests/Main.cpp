#define BOOST_TEST_MODULE ATPSearchTests


#include <fstream>
#include <iostream>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_monitor.hpp>
#include <boost/test/debug.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>


struct BoostDisableLoggingFixture
{
	BoostDisableLoggingFixture()
	{
		namespace logging = boost::log;

		logging::core::get()->set_filter
		(
			// disable logging
			logging::trivial::severity > logging::trivial::fatal
		);
	}
};


BOOST_GLOBAL_FIXTURE(BoostDisableLoggingFixture);


// Uncomment these lines if you want the test results to be written
// to a file instead of to the console:


//struct FileLoggingConfig
//{
//	FileLoggingConfig() : test_log("ATPLogicTests - results.txt")
//	{
//		boost::unit_test::unit_test_log.set_stream(test_log);
//	}
//	~FileLoggingConfig()
//	{
//		boost::unit_test::unit_test_log.set_stream(std::cout);
//	}
//
//	std::ofstream test_log;
//};
//
//
//BOOST_GLOBAL_FIXTURE(FileLoggingConfig);


