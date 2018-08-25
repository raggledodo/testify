#include "retroc/generate.hpp"

#ifdef GEN_HPP

std::string GenIO::get_string (std::string usage, size_t len,
	std::string content)
{
	auto out = ::get_string(len, content);
	testify::DescribedData* input = gcase_.mutable_inputs()->Add();
	input->set_usage(usage);
	input->set_dtype(serialize(input->mutable_data(),
		out.begin(), out.end()));
	return out;
}

std::vector<uint64_t> GenIO::choose (std::string usage, uint64_t n, uint64_t k)
{
	auto out = ::choose(n, k);
	testify::DescribedData* input = gcase_.mutable_inputs()->Add();
	input->set_usage(usage);
	input->set_dtype(serialize(input->mutable_data(),
		out.begin(), out.end()));
	return out;
}

void GenIO::send_case (void)
{
	if (gcase_.inputs().empty() || gcase_.outputs().empty())
	{
		throw std::runtime_error("sending empty case");
	}
	send(testname_, gcase_);
	gcase_.Clear();
}

#endif
