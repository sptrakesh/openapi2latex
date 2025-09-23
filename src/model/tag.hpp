//
// Created by Rakesh on 19/09/2025.
//

#pragma once

#include "external.hpp"

#include <optional>
#include <vector>

namespace spt::model
{
  struct Tag
  {
    std::optional<ExternalDocumentation> externalDocs;
    std::string name;
    std::string description;
  };

  struct TagGroup
  {
    std::vector<std::string> tags;
    std::string name;
  };
}