cmake_minimum_required(VERSION 3.0)
project (sig_finder)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")

# Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")

set (sig_srcs
	pattern_tree.cpp
)

set (sig_hdrs
	include/ShortList.h
	include/sig_finder.h
	include/pattern_tree.h
)
include_directories ( include )
add_library(${PROJECT_NAME} STATIC ${sig_srcs} ${sig_hdrs})
