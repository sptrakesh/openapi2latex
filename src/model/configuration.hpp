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
    std::string output{};
    std::string author{ "OpenAPI2LaTeX Generator" };
    std::string footer{ "Proprietary and Confidential" };
    std::string font{ "Helvetica Neue" };
    bool operationSummary = false;
    bool cmark = false;
  };
}