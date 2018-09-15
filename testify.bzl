load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

def _impl(ctx):
    ctx.download_and_extract(
        stripPrefix = "googletest-ec44c6c1675c25b9827aacd08c02433cccde7780",
        url = "https://github.com/google/googletest/archive/ec44c6c1675c25b9827aacd08c02433cccde7780.tar.gz",
    )

    ctx.file('BUILD', """
licenses(["notice"])

cc_library(
    name = "gtest",
    srcs = [
        "googletest/src/gtest-all.cc",
        "googlemock/src/gmock-all.cc",
    ],
    hdrs = glob([
        "**/*.h",
        "googletest/src/*.cc",
        "googlemock/src/*.cc",
    ]),
    includes = [
        "googlemock",
        "googletest",
        "googletest/include",
        "googlemock/include",
    ],
    linkopts = ["-pthread"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "gtest_main",
    srcs = ["googlemock/src/gmock_main.cc"],
    linkopts = ["-pthread"],
    visibility = ["//visibility:public"],
    deps = [":gtest"],
)
""")
    ctx.file('WORKSPACE', '')

get_gtest = repository_rule(implementation = _impl)

def dependencies():
    # protobuf dependency
    if "org_pubref_rules_protobuf" not in native.existing_rules():
        git_repository(
            name = "org_pubref_rules_protobuf",
            remote = "https://github.com/mingkaic/rules_protobuf",
            commit = "f5615fa9d544d0a69cd73d8716364d8bd310babe",
        )

    # gtest dependency
    if "com_google_googletest" not in native.existing_rules():
        get_gtest(name = "com_google_googletest")
