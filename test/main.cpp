//
// Created by Rakesh on 2025-09-22.
//

#include <catch2/catch_session.hpp>
#include "../src/log/NanoLog.hpp"

int main( int argc, char* argv[] )
{
  nanolog::initialize( nanolog::GuaranteedLogger(), "/tmp/", "oa2l-test", false );
  return Catch::Session().run( argc, argv );
}
