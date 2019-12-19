#pragma once


#include <Process.h>


// A basically empty process class for testing.
class DummyProcess :
	public atpsearch::IProcess
{
public:
	atpsearch::ProcessStatus tick() override
	{
		return atpsearch::status::not_finished();
	}

	void abort() override
	{ }

	std::string get_name() const override
	{
		return "";
	}

	std::string get_details() const override
	{
		return "";
	}
};


