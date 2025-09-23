//
// Created by Rakesh on 18/09/2025.
//

#pragma once

#include <boost/url/url.hpp>

namespace spt::model
{
  struct ExternalDocumentation
  {
    boost::urls::url url;
    std::string description;
  };
}