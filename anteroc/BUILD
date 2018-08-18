licenses(["notice"])

package(
    default_visibility = ["//visibility:public"],
)

filegroup(
    name = "srcs",
    srcs = glob([
        "*.hpp",
        "src/*.cpp"
    ]) + [
        "BUILD",
        "headers",
    ],
)

######### LIBRARY #########

cc_library(
    name = "anteroc",
    hdrs = glob(["*.hpp"]),
    srcs = glob(["src/*.cpp"]),
    copts = ["-std=c++14"],
    linkstatic = 1,
    deps = ["//proto:cc_proto"],
)

######### TEST #########

cc_test(
    name = "test",
    size = "small",
    srcs = ["test/main.cpp"],
    deps = [
        "//anteroc:anteroc",
        "@com_google_googletest//:gtest",
    ],
    linkstatic = 1,
    copts = ["-std=c++14"],
)
