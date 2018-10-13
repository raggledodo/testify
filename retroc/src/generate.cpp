#include "retroc/generate.hpp"

#ifdef RETRO_GEN_HPP

namespace retro
{

std::string GenIO::get_string (std::string usage, size_t len,
	std::string content)
{
	auto out = retro::get_string(len, content);
	testify::CaseData input;
	serialize(input, out.begin(), out.end());
	gcase_.mutable_inputs()->insert({usage, input});
	return out;
}

std::vector<uint64_t> GenIO::choose (std::string usage, uint64_t n, uint64_t k)
{
	auto out = retro::choose(n, k);
	testify::CaseData input;
	serialize(input, out.begin(), out.end());
	gcase_.mutable_inputs()->insert({usage, input});
	return out;
}

void GenIO::send_case (void)
{
	if (gcase_.inputs().empty())
	{
		throw std::runtime_error("sending empty case");
	}
	retro::send(testname_, gcase_);
	gcase_.Clear();
}

}

#endif
