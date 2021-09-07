# -*- Python -*-

# Configuration file for the 'lit' test runner.

import platform

import lit.formats
# Global instance of LLVMConfig provided by lit
from lit.llvm import llvm_config
from lit.llvm.subst import ToolSubst

# name: The name of this test suite.
# (config is an instance of TestingConfig created when discovering tests)
config.name = 'lartcc'

# testFormat: The test format to use to interpret tests.
config.test_format = lit.formats.ShTest(not llvm_config.use_lit_shell)

# suffixes: A list of file extensions to treat as test files. This is overriden
# by individual lit.local.cfg files in the test subdirectories.
config.suffixes = ['.ll', '.c', '.cpp']

# test_source_root: The root path where tests are located.
config.test_source_root = os.path.dirname(__file__)

config.test_exec_root = os.path.join(config.my_obj_root, 'test')

# excludes: A list of directories to exclude from the testsuite. The 'Inputs'
# subdirectories contain auxiliary inputs for various tests in their parent
# directories.
config.excludes = []

tool_substitutions = [
    ToolSubst('%lartcc', os.path.join(config.my_obj_root, '../lartcc/lartcc')),
    ToolSubst('%lartcc++', os.path.join(config.my_obj_root, '../lartcc/lartcc')),
    ToolSubst('%testrun', os.path.join(config.my_obj_root, 'scripts/testrun.sh'))
]

llvm_config.add_tool_substitutions(tool_substitutions)

# The list of tools required for testing - prepend them with the path specified
# during configuration (i.e. LLVM_TOOLS_DIR/bin)
tools = ["opt", "lli", "not", "FileCheck", "clang"]
llvm_config.add_tool_substitutions(tools, config.llvm_tools_dir)

# The LIT variable to hold the file extension for shared libraries (this is
# platform dependent)
config.substitutions.append(('%shlibext', config.llvm_shlib_ext))
# The LIT variable to hold the location of plugins/libraries
config.substitutions.append(('%shlibdir', config.llvm_shlib_dir))


