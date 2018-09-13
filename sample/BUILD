licenses(["notice"])

filegroup(
    name = "sample",
    srcs = ["sample.pb"],
    visibility = ["//visibility:public"],
)

py_binary(
    name = "gen",
    srcs = ["gen.py"],
    deps = ["//retrop:retrop"],
)
