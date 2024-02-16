cmake_minimum_required ( VERSION 3.0 )

project ( SigFinder )

option(SIGFINDER_BUILD_TESTING "Enable testing for SigFinder" ON)

# modules:
set ( M_SIGFINDER_LIB "sig_finder" )
set ( M_SIGFINDER_DEMO "demo" )

# modules paths:
set ( PARAMKIT_DIR "${CMAKE_SOURCE_DIR}/${M_SIGFINDER_LIB}" CACHE PATH "SigFinder main path" )

# Add sub-directories
#
# libs
add_subdirectory ( sig_finder )
set ( PARAMKIT_LIB $<TARGET_FILE:sig_finder> CACHE PATH "SigFinder library path" )

# Setup testing
if(SIGFINDER_BUILD_TESTING)
	enable_testing()
	
	#demos:
	add_subdirectory ( ${M_SIGFINDER_DEMO} )
	add_dependencies ( ${M_SIGFINDER_DEMO} sig_finder )
endif()
