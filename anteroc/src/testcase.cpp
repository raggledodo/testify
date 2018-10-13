#include "anteroc/testcase.hpp"

static std::unordered_map<std::string,
	std::list<testify::GeneratedCase> > cases_;

testify::GeneratedCase Testament::get (std::string testname)
{
	auto it = cases_.find(testname);
	if (cases_.end() == it)
	{
		std::vector<testify::GeneratedCase> upvec;
		bool good = antero::get_cases(upvec, testname);
		if (false == good || upvec.empty())
		{
			throw std::runtime_error("testcase returned empty: " + testname);
		}
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
