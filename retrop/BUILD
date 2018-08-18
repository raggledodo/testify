licenses(["notice"])

package(
    default_visibility = ["//visibility:public"],
)

filegroup(
    name = "srcs",
    srcs = glob(["*.py"]) + ["BUILD"],
)

######### LIBRARY #########

py_library(
    name = "retrop",
    srcs = glob(["*.py"], exclude = ["test_client.py", "test_rand.py"]),
    deps = ["//proto:py_proto"],
)

######### TEST #########

py_test(
    name = "test_client",
    size = "enormous",
    srcs = ["test_client.py"],
    deps = ["//retrop:retrop"],
)

py_test(
    name = "test_rand",
    size = "enormous",
    srcs = ["test_rand.py"],
    deps = ["//retrop:retrop"],
)
