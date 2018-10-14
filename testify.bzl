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
    if "com_github_raggledodo_dora" not in native.existing_rules():
        git_repository(
            name = "com_github_raggledodo_dora",
            remote = "https://github.com/raggledodo/dora",
            commit = "9765cc910e2bc967d65d7a88db260d57da83f1fd",
        )

    # gtest dependency
    if "com_google_googletest" not in native.existing_rules():
        get_gtest(name = "com_google_googletest")
