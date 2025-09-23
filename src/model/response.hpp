//
// Created by Rakesh on 19/09/2025.
//

#pragma once

#include "header.hpp"
#include "link.hpp"
#include "mediatype.hpp"

namespace spt::model
{
  struct Response
  {
    std::map<std::string, Header, std::less<>> headers;
    std::map<std::string, MediaType, std::less<>> content;
    std::map<std::string, Link, std::less<>> links;
    std::string _referenceURI;
    std::string ref;
    std::string description;
  };
}