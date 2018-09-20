GTEST_REPEAT := 50

COMMON_BZL_FLAGS := --test_output=all --cache_test_results=no

GTEST_FLAGS := --action_env="GTEST_SHUFFLE=1" --action_env="GTEST_BREAK_ON_FAILURE=1"

REP_BZL_FLAGS := --action_env="GTEST_REPEAT=$(GTEST_REPEAT)"

VAL_BZL_FLAGS := --run_under="valgrind --leak-check=full"

ASAN_BZL_FLAGS := --linkopt -fsanitize=address

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

valgrind: valgrind_retroc valgrind_jack

valgrind_retroc: valgrind_retroc_client valgrind_retroc_rand

valgrind_retroc_client:
	$(GTEST) $(VAL_BZL_FLAGS) @com_github_mingkaic_testify//retroc:test_client

valgrind_retroc_rand:
	$(GTEST) $(VAL_BZL_FLAGS) --action_env="GTEST_REPEAT=3" @com_github_mingkaic_testify//retroc:test_rand

valgrind_jack: valgrind_jackgen valgrind_jackread valgrind_jackoffline

valgrind_jackgen:
	$(GTEST) $(VAL_BZL_FLAGS) --action_env="GTEST_REPEAT=15" @com_github_mingkaic_testify//simple:test_gen

valgrind_jackread:
	$(GTEST) $(VAL_BZL_FLAGS) --action_env="GTEST_REPEAT=15" @com_github_mingkaic_testify//simple:test_read

valgrind_jackoffline: valgrind_jackgenoffline valgrind_jackreadoffline

valgrind_jackgenoffline:
	$(GTEST) $(VAL_BZL_FLAGS) --action_env="GTEST_REPEAT=15" @com_github_mingkaic_testify//simple:test_genoffline

valgrind_jackreadoffline:
	$(GTEST) $(VAL_BZL_FLAGS) --action_env="GTEST_REPEAT=15" @com_github_mingkaic_testify//simple:test_readoffline

# asan unit tests

asan: asan_retroc asan_jack

asan_retroc: asan_retroc_client asan_retroc_rand

asan_retroc_client:
	$(GTEST) $(ASAN_BZL_FLAGS) @com_github_mingkaic_testify//retroc:test_client

asan_retroc_rand:
	$(GTEST) $(ASAN_BZL_FLAGS) $(REP_BZL_FLAGS) @com_github_mingkaic_testify//retroc:test_rand

asan_jack: asan_jackgen asan_jackread asan_jackoffline

asan_jackgen:
	$(GTEST) $(ASAN_BZL_FLAGS) $(REP_BZL_FLAGS) @com_github_mingkaic_testify//simple:test_gen

asan_jackread:
	$(GTEST) $(ASAN_BZL_FLAGS) $(REP_BZL_FLAGS) @com_github_mingkaic_testify//simple:test_read

asan_jackoffline: asan_jackgenoffline asan_jackreadoffline

asan_jackgenoffline:
	$(GTEST) $(ASAN_BZL_FLAGS) $(REP_BZL_FLAGS) @com_github_mingkaic_testify//simple:test_genoffline

asan_jackreadoffline:
	$(GTEST) $(ASAN_BZL_FLAGS) $(REP_BZL_FLAGS) @com_github_mingkaic_testify//simple:test_readoffline
