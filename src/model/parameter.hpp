//
// Created by Rakesh on 18/09/2025.
//

#pragma once

#include "example.hpp"

#include <map>

namespace spt::model
{
  struct Parameter
  {
    std::map<std::string, Example, std::less<>> examples;
    std::any example;
    std::string _referenceURI;
    std::string ref;
    std::string name;
    std::string in;
    std::string description;
    std::string style;
    bool required;
    bool deprecated;
    bool allowEmptyValue;
    bool explode;
    bool allowReserved;
  };
}