#include "anteroc/testcase.hpp"

static std::unordered_map<std::string,
	std::list<testify::GeneratedCase> > cases_;

testify::GeneratedCase Testament::get (std::string testname)
{
	auto it = cases_.find(testname);
	if (cases_.end() == it)
	{
		auto upout = antero::get_cases(testname);
		if (false == (bool) upout || upout->empty())
		{
			throw std::runtime_error("testcase returned empty: " + testname);
		}
		auto upvec = *upout;
		cases_[testname] =
			std::list<testify::GeneratedCase>(upvec.begin(), upvec.end());
	}
	auto out = cases_[testname].front();
	cases_[testname].pop_front();
	if (cases_[testname].empty())
	{
		cases_.erase(testname);
	}
	return out;
}
