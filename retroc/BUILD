licenses(["notice"])

package(
    default_visibility = ["//visibility:public"],
)

filegroup(
    name = "srcs",
    srcs = glob(["src/*.cpp"]) + [
        "BUILD",
        "headers",
    ],
)

filegroup(
    name = "headers",
    srcs = glob([
        "*.hpp",
        "include/*.hpp",
        "include/*.ipp",
    ])
)

######### LIBRARY #########

cc_library(
    name = "retroc",
    hdrs = ["headers"],
    srcs = glob(["src/*.cpp"]),
    copts = ["-std=c++14"],
    linkstatic = 1,
    deps = ["//proto:cc_proto"],
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
