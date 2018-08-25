#include "anteroc/testcase.hpp"

static std::unordered_map<std::string,
    std::list<testify::GeneratedCase> > outputs_;

testify::GeneratedCase testament::get (std::string testname)
{
    auto it = outputs_.find(testname);
    if (outputs_.end() == it)
    {
        auto upout = get_outputs(testname);
        if (upout.empty())
        {
            throw std::exception();
        }
        outputs_[testname] =
            std::list<testify::GeneratedCase>(upout.begin(), upout.end());
    }
    auto out = outputs_[testname].front();
    outputs_[testname].pop_front();
    if (outputs_[testname].empty())
    {
        outputs_.erase(testname);
    }
    return out;
}
