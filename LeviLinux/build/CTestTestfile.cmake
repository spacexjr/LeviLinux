# CMake generated Testfile for 
# Source directory: /home/re/Documentos/LeviLinux/LeviLinux
# Build directory: /home/re/Documentos/LeviLinux/LeviLinux/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(TestCoreEngine "/home/re/Documentos/LeviLinux/LeviLinux/build/test_core_engine")
set_tests_properties(TestCoreEngine PROPERTIES  _BACKTRACE_TRIPLES "/home/re/Documentos/LeviLinux/LeviLinux/CMakeLists.txt;90;add_test;/home/re/Documentos/LeviLinux/LeviLinux/CMakeLists.txt;0;")
subdirs("src/core")
subdirs("src/loader")
subdirs("src/ui")
