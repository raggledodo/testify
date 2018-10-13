#include <algorithm>
#include <sstream>

#include "retroc/rand.hpp"

#ifdef RETRO_RAND_HPP

namespace retro
{

EngineT& get_engine (void)
{
	static EngineT engine;
	return engine;
}

std::string get_string (size_t len, std::string content)
{
	std::string out(len, 0);
	get_vec(out.begin(), out.end(), content.begin(), content.end());
	return out;
}

std::vector<uint64_t> choose (uint64_t n, uint64_t k)
{
	if (k > n)
	{
		std::stringstream ss;
		ss << "choosing larger k=" << k << " over n=" << n;
		throw std::runtime_error(ss.str());
	}

	std::vector<uint64_t> output(n, 0);
	std::iota(output.begin(), output.end(), 0);
	std::shuffle(output.begin(), output.end(), get_engine());
	auto it = output.begin();
	return std::vector<uint64_t>(it, it + k);
}

}

#endif
