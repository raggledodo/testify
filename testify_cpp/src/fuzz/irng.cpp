#include "testify_cpp/include/fuzz/irng.hpp"

#ifdef TESTIFY_IRNG_HPP

namespace testify
{

static RNG_F generator;

std::default_random_engine& get_generator (void)
{
	assert(generator);
	return generator();
}

void set_generator (RNG_F gen)
{
	generator = gen;
}

}

#endif
