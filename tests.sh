#!/usr/bin/env bash

# ===== Run Gtest =====
echo "===== TESTS =====";

make
make valgrind

echo "";
echo "============ TESTIFY TEST SUCCESSFUL ============";
