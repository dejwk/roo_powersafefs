load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "roo_powersafefs",
    srcs = [
        "src/roo_powersafefs.cpp",
        "src/roo_powersafefs.h",
    ],
    includes = [
        "src",
    ],
    visibility = ["//visibility:public"],
    deps = ["@roo_threads"],
)
