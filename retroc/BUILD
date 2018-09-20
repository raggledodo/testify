licenses(["notice"])

package(
    default_visibility = ["//visibility:public"],
)

filegroup(
    name = "srcs",
    srcs = glob([
        "*.hpp",
        "src/*.cpp",
    ]) + ["BUILD"],
)

######### LIBRARY #########

cc_library(
    name = "retroc",
    hdrs = glob(["*.hpp"]),
    srcs = glob(["src/*.cpp"]),
    copts = ["-std=c++14"],
    deps = ["//client:cpp_client"],
)

######### TEST #########

cc_test(
    name = "test_client",
    size = "small",
    srcs = ["test/client.cpp"],
    deps = [
        "//retroc:retroc",
        "@com_google_googletest//:gtest",
    ],
    linkstatic = 1,
    copts = ["-std=c++14"],
)

cc_test(
    name = "test_rand",
    size = "small",
    srcs = ["test/rand.cpp"],
    deps = [
        "//retroc:retroc",
        "@com_google_googletest//:gtest",
    ],
    linkstatic = 1,
    copts = ["-std=c++14"],
)
