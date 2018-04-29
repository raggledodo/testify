def dependencies():
    # gtest dependency
    if "com_google_googletest" not in native.existing_rules():
        native.new_http_archive(
            name = "com_google_googletest",
            build_file = "@com_github_mingkaic_testify//third_party:gtest.BUILD",
            strip_prefix = "googletest-ec44c6c1675c25b9827aacd08c02433cccde7780",
            url = "https://github.com/google/googletest/archive/ec44c6c1675c25b9827aacd08c02433cccde7780.tar.gz",
        )
