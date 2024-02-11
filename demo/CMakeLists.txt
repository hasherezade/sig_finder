cmake_minimum_required ( VERSION 3.0 )

project (demo)

message (STATUS "sig_finder_dir='${PARAMKIT_DIR}'")
message (STATUS "sig_finder_lib='${PARAMKIT_LIB}'")

include_directories ( ${PARAMKIT_DIR}/include )

set (srcs
	main.cpp
)

set (hdrs
)

add_executable ( ${PROJECT_NAME} ${hdrs} ${srcs} )
target_link_libraries ( ${PROJECT_NAME} sig_finder )
add_dependencies( ${PROJECT_NAME} sig_finder )

INSTALL( TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX} COMPONENT ${PROJECT_NAME} )
