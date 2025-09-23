Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.10.0
)

FetchContent_MakeAvailable(Catch2)

file(GLOB_RECURSE test_SRCS *.cpp *.h)
add_executable(unitTest ${test_SRCS} )

target_link_libraries(unitTest
  PRIVATE oa-lib Boost::url Catch2::Catch2
)

