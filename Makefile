COMMON_BZL_FLAGS := --test_output=all --cache_test_results=no

GTEST_FLAGS := --action_env="GTEST_REPEAT=25" \
	--action_env="GTEST_SHUFFLE=1" --action_env="GTEST_BREAK_ON_FAILURE=1"

TEST := bazel test $(COMMON_BZL_FLAGS)

GTEST := $(TEST) $(GTEST_FLAGS)

all: test

test: cpp_test py_test

cpp_test:
	$(GTEST) //testify_cpp:test
	
py_test: py_build graphast_test tfgen_test

graphast_test:
	$(TEST) //testify_py:graphast_test

tfgen_test:
	$(TEST) //testify_py:tfgen_test
