licenses(["notice"])

package(
    default_visibility = [ "//visibility:public" ]
)

filegroup(
    name = "srcs",
    srcs = glob([
        "include/**/*.hpp",
        "src/**/*.cpp",
        "tests/**/*.cpp",
    ]) + [
        "BUILD",
    ],
)

# LIBRARY

cc_library(
    name = "testify",
    hdrs = glob([ "include/**/*.hpp" ]),
    srcs = glob([ "src/**/*.cpp" ]),
    deps = [ "@com_google_googletest//:gtest" ],
    copts = [ "-std=c++14" ],
    linkstatic = 1,
)

# TEST

cc_test(
    name = "test",
    srcs = [
        "tests/test_fuzz.cpp",
        "tests/test_mocker.cpp",
        "tests/test_rng.cpp",
    ],
    deps = [
        ":testify",
        "@com_google_googletest//:gtest_main",
    ],
    copts = [ "-std=c++14" ],
)
