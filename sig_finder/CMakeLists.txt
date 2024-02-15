cmake_minimum_required(VERSION 3.0)
project (sig_finder)

# Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")

set (sig_srcs
	Signature.cpp
	Node.cpp
	Match.cpp
)

set (sig_hdrs
	include/ShortList.h
	include/Signature.h
	include/Match.h
	include/Node.h
	include/find_helpers.h
	include/sig_finder.h
)
include_directories ( include )
add_library(${PROJECT_NAME} STATIC ${sig_srcs} ${sig_hdrs})
