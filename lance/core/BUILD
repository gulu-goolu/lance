cc_library(
    name = "core",
    srcs = [
        "file_system.cc",
        "object.cc",
    ],
    hdrs = [
        "file_system.h",
        "linalg.h",
        "object.h",
        "util.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_test(
    name = "unittests",
    srcs = ["util_test.cc"],
    deps = [
        ":core",
        "@com_google_googletest//:gtest_main",
    ],
)
