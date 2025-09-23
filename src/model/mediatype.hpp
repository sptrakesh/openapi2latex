//
// Created by Rakesh on 18/09/2025.
//

#pragma once

#include "example.hpp"
#include "header.hpp"
#include "schema.hpp"

#include <map>

namespace spt::model
{
  struct MediaType
  {
    struct Encoding
    {
      std::map<std::string, Header, std::less<>> headers;
      std::string contentType;
      std::string style;
      bool explode;
      bool allowReserved;
    };

    std::map<std::string, Example, std::less<>> examples;
    std::map<std::string, Encoding, std::less<>> encoding;
    std::any example;
    std::optional<Schema> schema;
  };
}