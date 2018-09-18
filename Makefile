GTEST_REPEAT := 50

COMMON_BZL_FLAGS := --test_output=all --cache_test_results=no

GTEST_FLAGS := --action_env="GTEST_SHUFFLE=1" --action_env="GTEST_BREAK_ON_FAILURE=1"

REP_BZL_FLAGS := --action_env="GTEST_REPEAT=$(GTEST_REPEAT)"

VAL_BZL_FLAGS := --run_under="valgrind --leak-check=full"

TEST := bazel test $(COMMON_BZL_FLAGS)

GTEST := $(TEST) $(GTEST_FLAGS)

# all tests

test: test_retroc test_retrop test_anteroc test_jack

test_retroc: test_retroc_client test_retroc_rand

test_retroc_client:
	$(GTEST) @com_github_mingkaic_testify//retroc:test_client

test_retroc_rand:
	$(GTEST) $(REP_BZL_FLAGS) @com_github_mingkaic_testify//retroc:test_rand

test_retrop: test_retrop_client test_retrop_rand

test_retrop_client:
	$(TEST) @com_github_mingkaic_testify//retrop:test_client

test_retrop_rand:
	$(TEST) --action_env="PYTEST_REPEAT=50" @com_github_mingkaic_testify//retrop:test_rand

test_anteroc:
	$(GTEST) $(REP_BZL_FLAGS) @com_github_mingkaic_testify//anteroc:test

test_jack: test_jackgen test_jackread test_jackoffline

test_jackgen:
	$(GTEST) $(REP_BZL_FLAGS) @com_github_mingkaic_testify//simple:test_gen

test_jackread:
	$(GTEST) $(REP_BZL_FLAGS) @com_github_mingkaic_testify//simple:test_read

test_jackoffline: test_jackgenoffline test_jackreadoffline

test_jackgenoffline:
	$(GTEST) $(REP_BZL_FLAGS) @com_github_mingkaic_testify//simple:test_genoffline

test_jackreadoffline:
	$(GTEST) $(REP_BZL_FLAGS) @com_github_mingkaic_testify//simple:test_readoffline

# valgrind unit tests

valgrind_jack: valgrind_jackgen valgrind_jackread valgrind_jackoffline

valgrind_jackgen:
	$(GTEST) $(VAL_BZL_FLAGS) @com_github_mingkaic_testify//simple:test_gen

valgrind_jackread:
	$(GTEST) $(VAL_BZL_FLAGS) @com_github_mingkaic_testify//simple:test_read

valgrind_jackoffline: valgrind_jackgenoffline valgrind_jackreadoffline

valgrind_jackgenoffline:
	$(GTEST) $(VAL_BZL_FLAGS) @com_github_mingkaic_testify//simple:test_genoffline

valgrind_jackreadoffline:
	$(GTEST) $(VAL_BZL_FLAGS) @com_github_mingkaic_testify//simple:test_readoffline
