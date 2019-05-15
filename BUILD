
cc_library(
  name = "crack",
  srcs = [
      "source/crack.cc",
      "source/md4.cc"
  ],
  hdrs = [
      "source/crack.h",
      "source/md4.h",
  ]
)

cc_binary(
  name = "crack_md4",
  srcs = ["source/main.cc"],
  deps = [":crack"],
)
