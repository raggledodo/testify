load("@org_pubref_rules_protobuf//cpp:rules.bzl", "cc_proto_library")
load("@org_pubref_rules_protobuf//python:rules.bzl", "py_proto_library")

licenses(["notice"])

package(default_visibility = ["//visibility:public"])

filegroup(
    name = "srcs",
    srcs = ["BUILD", "protos"],
)

filegroup(
    name = "protos",
    srcs = glob(["*.proto"]),
)

######### LIBRARY #########

proto_library(
    name = "proto",
    srcs = ["protos"],
    deps = ["@com_google_protobuf//:any_proto"],
)

cc_proto_library(
    name = "cc_proto",
    protos = ["protos"],
    imports = ["external/com_google_protobuf/src"],
    inputs = ["@com_google_protobuf//:well_known_protos"],
    with_grpc = True,
)

py_proto_library(
    name = "py_proto",
    protos = ["protos"],
    imports = ["external/com_google_protobuf/src"],
    inputs = ["@com_google_protobuf//:well_known_protos"],
    with_grpc = True,
)
