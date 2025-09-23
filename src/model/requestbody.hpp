//
// Created by Rakesh on 19/09/2025.
//

#pragma once

#include "mediatype.hpp"

namespace spt::model
{
  struct RequestBody
  {
    std::map<std::string, MediaType, std::less<>> content;
    std::string _referenceURI;
    std::string ref;
    std::string description;
    bool required;
  };
}