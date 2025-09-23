//
// Created by Rakesh on 23/09/2025.
//

#pragma once

#include <string>

namespace spt::model
{
  struct Configuration
  {
    std::string input{};
    std::string output{"/tmp/"};
    std::string author{ "OpenAPI2LaTeX Generator" };
    std::string footer{ "Proprietary and Confidential" };
    bool operationSummary = false;
    bool cmark = false;
  };
}