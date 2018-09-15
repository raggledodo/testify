COMMON_BZL_FLAGS := --test_output=all --cache_test_results=no

GTEST_FLAGS := --action_env="GTEST_SHUFFLE=1" --action_env="GTEST_BREAK_ON_FAILURE=1"

TEST := bazel test $(COMMON_BZL_FLAGS)

all: test_retroc test_retrop test_anteroc test_jack

test_retroc: test_retroc_client test_retroc_rand

test_retroc_client:
	$(TEST) $(GTEST_FLAGS) @com_github_mingkaic_testify//retroc:test_client

test_retroc_rand:
	$(TEST) $(GTEST_FLAGS) --action_env="GTEST_REPEAT=50" @com_github_mingkaic_testify//retroc:test_rand

test_retrop: test_retrop_client test_retrop_rand

test_retrop_client:
	$(TEST) @com_github_mingkaic_testify//retrop:test_client

test_retrop_rand:
	$(TEST) --action_env="PYTEST_REPEAT=50" @com_github_mingkaic_testify//retrop:test_rand

test_anteroc:
	$(TEST) $(GTEST_FLAGS) --action_env="GTEST_REPEAT=50" @com_github_mingkaic_testify//anteroc:test

test_jack:
	$(TEST) $(GTEST_FLAGS) @com_github_mingkaic_testify//simple:test_gen
	$(TEST) $(GTEST_FLAGS) @com_github_mingkaic_testify//simple:test_read
