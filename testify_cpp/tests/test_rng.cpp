#include "gtest/gtest.h"

#include "testify_cpp/include/fuzz/irng.hpp"


TEST(RNG, SetRng)
{
	std::uniform_real_distribution<double> dist;
	testify::set_generator(
	[](void) -> std::default_random_engine&
	{
		static std::default_random_engine common_gen(12);
		return common_gen;
	});
	std::default_random_engine& engine1 = testify::get_generator();
	testify::set_generator(
	[](void) -> std::default_random_engine&
	{
		static std::default_random_engine common_gen(12);
		return common_gen;
	});
	std::default_random_engine& engine2 = testify::get_generator();
	EXPECT_EQ(dist(engine1), dist(engine2));
}
