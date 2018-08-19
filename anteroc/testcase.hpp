#include "gtest/gtest.h"

#include "anteroc/client.hpp"

class testament : public ::testing::Test
{
protected:
    testify::TestOutput get (std::string testname);
};
