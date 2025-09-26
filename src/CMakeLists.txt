file(GLOB_RECURSE oa2l_SRCS *.cpp *.hpp)

include(FetchContent)
FetchContent_Declare(ryml
  GIT_REPOSITORY https://github.com/biojppm/rapidyaml.git
  GIT_TAG master
  GIT_SHALLOW FALSE  # ensure submodules are checked out
)
FetchContent_MakeAvailable(ryml)

if (BUILD_TESTING)
  file(GLOB_RECURSE oa2l-lib_SRCS log/* model/* output/* util/split.hpp)
  add_library(oa-lib STATIC ${oa2l-lib_SRCS})
  target_link_libraries(oa-lib PRIVATE Boost::url ryml::ryml)
endif()

add_executable(oa2tex ${oa2l_SRCS} )
target_link_libraries(oa2tex PRIVATE Boost::url ryml::ryml)

install(TARGETS oa2tex DESTINATION bin)
