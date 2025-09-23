//
// Created by Rakesh on 22/09/2025.
//

#pragma once

#include "../model/configuration.hpp"
#include "../model/openapi.hpp"

namespace spt::output
{
  std::string generate( model::OpenAPI& openapi, const model::Configuration& config );
  std::string convert( std::string_view data );
}