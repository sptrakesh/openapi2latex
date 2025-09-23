//
// Created by Rakesh on 18/09/2025.
//

#pragma once

#include <map>
#include <vector>
#include <boost/url/url.hpp>

namespace spt::model
{
  struct Server
  {
    struct ServerVariable
    {
      std::vector<std::string> _enum;
      std::string _default;
      std::string description;
    };

    std::map<std::string, ServerVariable, std::less<>> variables;
    boost::urls::url url;
    std::string description;
  };
}