# Configuration file for the 'lit' test runner.
import os
import lit.formats

config.name = 'LLVM-EXPLORE'
config.test_format = lit.formats.ShTest("0")
config.suffixes = ['.ll']
config.test_source_root = os.path.dirname(__file__)
