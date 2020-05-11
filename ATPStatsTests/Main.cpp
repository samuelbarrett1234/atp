#define BOOST_TEST_MODULE ATPStatsTests


#include <fstream>
#include <iostream>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_monitor.hpp>
#include <boost/test/debug.hpp>


// Uncomment these lines if you want the test results to be written
// to a file instead of to the console:


//struct FileLoggingConfig
//{
//	FileLoggingConfig() : test_log("ATPStatsTests - results.txt")
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


