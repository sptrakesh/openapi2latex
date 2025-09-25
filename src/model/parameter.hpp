//
// Created by Rakesh on 18/09/2025.
//

#pragma once

#include "example.hpp"
#include "schema.hpp"

#include <map>

namespace spt::model
{
  struct Parameter
  {
    std::optional<Schema> schema;
    std::map<std::string, Example, std::less<>> examples;
    std::any example;
    std::string _referenceURI;
    std::string ref;
    std::string name;
    std::string in;
    std::string description;
    std::string style;
    bool required = false;
    bool deprecated = false;
    bool allowEmptyValue = false;
    bool explode = false;
    bool allowReserved = false;
  };
}