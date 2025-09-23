//
// Created by Rakesh on 18/09/2025.
//

#pragma once

#include <optional>
#include <boost/url/url.hpp>

namespace spt::model
{
  struct Info
  {
    struct Contact
    {
      boost::urls::url url;
      std::string name;
      std::string email;
    };

    struct License
    {
      std::string name;
      std::string identifier;
      boost::urls::url url;
    };

    std::optional<Contact> contact;
    std::optional<License> license;
    boost::urls::url termsOfService;
    std::string title;
    std::string summary;
    std::string description;
    std::string version;
  };
}