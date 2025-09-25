//
// Created by Rakesh on 18/09/2025.
//

#pragma once

#include "schema.hpp"

namespace spt::model
{
  struct Example;
  struct MediaType;

  struct Header
  {
    std::map<std::string, Example, std::less<>> examples;
    std::map<std::string, MediaType, std::less<>> content;
    std::optional<Schema> schema;
    std::any example;
    std::string description;
    std::string style;
    bool required = false;
    bool deprecated = false;
    bool allowEmptyValue = false;
    bool explode = false;
  };
}