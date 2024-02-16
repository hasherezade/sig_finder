cmake_minimum_required ( VERSION 3.0 )

project (demo)

message (STATUS "sig_finder_dir='${PARAMKIT_DIR}'")
message (STATUS "sig_finder_lib='${PARAMKIT_LIB}'")

include_directories ( ${PARAMKIT_DIR}/include )

set (srcs
	main.cpp
)

set (hdrs
	util.h
	code_patterns.h
)

add_executable ( ${PROJECT_NAME} ${hdrs} ${srcs} )
target_link_libraries ( ${PROJECT_NAME} sig_finder )
add_dependencies( ${PROJECT_NAME} sig_finder )

#install
INSTALL( TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX} COMPONENT ${PROJECT_NAME} )

# Setup testing

enable_testing()
	
# 0) Basic Test (Aho-Corasic patterns, etc)
add_test (TestBasic ${CMAKE_INSTALL_PREFIX}/demo )
set_tests_properties(TestBasic PROPERTIES PASS_REGULAR_EXPRESSION "All passed")

# 1) SigFinder vs Naive search
add_test (TestCompareWithNaive_Self ${CMAKE_INSTALL_PREFIX}/demo ${CMAKE_INSTALL_PREFIX}/demo.exe)
set_tests_properties(TestCompareWithNaive_Self PROPERTIES FAIL_REGULAR_EXPRESSION "Test failed")

# 2) SigFinder vs Naive search (ntdll.dll)
add_test (TestCompareWithNaive_Ntdll ${CMAKE_INSTALL_PREFIX}/demo C:/Windows/System32/ntdll.dll)
set_tests_properties(TestCompareWithNaive_Ntdll PROPERTIES FAIL_REGULAR_EXPRESSION "Test failed")
set_tests_properties(TestCompareWithNaive_Explorer PROPERTIES SKIP_REGULAR_EXPRESSION "Failed to load")

# 3) SigFinder vs Naive search (ntoskrnl.exe)
add_test (TestCompareWithNaive_Ntoskrnl ${CMAKE_INSTALL_PREFIX}/demo C:/Windows/System32/ntoskrnl.exe)
set_tests_properties(TestCompareWithNaive_Ntoskrnl PROPERTIES FAIL_REGULAR_EXPRESSION "Test failed")
set_tests_properties(TestCompareWithNaive_Ntoskrnl PROPERTIES SKIP_REGULAR_EXPRESSION "Failed to load")

# 4) SigFinder vs Naive search (explorer.exe)
add_test (TestCompareWithNaive_Explorer ${CMAKE_INSTALL_PREFIX}/demo C:/Windows/explorer.exe)
set_tests_properties(TestCompareWithNaive_Explorer PROPERTIES FAIL_REGULAR_EXPRESSION "Test failed")
set_tests_properties(TestCompareWithNaive_Explorer PROPERTIES SKIP_REGULAR_EXPRESSION "Failed to load")
