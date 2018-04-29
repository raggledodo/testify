#include <cassert>
#include <random>
#include <functional>

#ifndef TESTIFY_IRNG_HPP
#define TESTIFY_IRNG_HPP

namespace testify
{
	
using RNG_F = std::function<std::default_random_engine&()>;

std::default_random_engine& get_generator (void);

void set_generator (RNG_F gen);

}

#endif /* TESTIFY_IRNG_HPP */
