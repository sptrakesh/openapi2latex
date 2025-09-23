file(GLOB_RECURSE oa2l_SRCS *.cpp *.hpp)

if (BUILD_TESTING)
  file(GLOB_RECURSE oa2l-lib_SRCS log/* model/* output/* util/split.hpp)
  add_library(oa-lib STATIC ${oa2l-lib_SRCS})
  target_link_libraries(oa-lib PRIVATE Boost::url)
endif()

add_executable(oa2tex ${oa2l_SRCS} )
target_link_libraries(oa2tex PRIVATE Boost::url)

install(TARGETS oa2tex DESTINATION bin)
