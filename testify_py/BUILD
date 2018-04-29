licenses(["notice"])

package(
    default_visibility = [ "//visibility:public" ],
)

filegroup(
    name = "srcs",
    srcs = glob([
        "shapeclass/*.py",
        "graphast/*.py",
        "tfgen/**/*.py",
        "tests/**/*.py",
    ]) + [
        "BUILD",
        "graphast/structure.yml",
    ],
)

# LIBRARIES

py_library(
    name = "shapeclass",
    srcs = glob([ "shapeclass/*.py" ]),
    imports = [ "." ],
)

py_library(
    name = "graphast",
    srcs = glob([ "graphast/*.py" ]),
    data = [ "graphast/structure.yml" ],
    deps = [ ":shapeclass" ],
    imports = [ "." ],
)

py_library(
    name = "tfgen",
    srcs = glob([ "tfgen/*.py" ]),
    deps = [ ":graphast" ],
    imports = [ "." ],
)

# TESTS

py_test(
    name = "graphast_test",
    main = "tests/graphast/test.py",
    srcs = glob([ "tests/graphast/*.py" ]),
    deps = [ ":graphast" ],
)

py_test(
    name = "tfgen_test",
    main = "tests/tfgen/tfgen_test.py",
    srcs = glob([ "tests/tfgen/*.py" ]),
    deps = [ ":graphast", ":tfgen" ],
    size = "enormous",
)
