//
// Created by Rakesh on 18/09/2025.
//

#pragma once

#include <any>
#include <boost/url/url.hpp>

namespace spt::model
{
  struct Example
  {
    boost::urls::url externalValue;
    std::any value;
    std::string summary;
    std::string description;
  };
}