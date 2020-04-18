set(TEST_SITE_CFG_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.py.in")
set(TEST_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

configure_file("${TEST_SITE_CFG_INPUT}"
  "${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py" @ONLY
)
