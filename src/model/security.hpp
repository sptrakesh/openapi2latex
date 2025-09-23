//
// Created by Rakesh on 19/09/2025.
//

#pragma once

#include <map>
#include <vector>
#include <boost/url/url.hpp>

namespace spt::model
{
  struct SecurityRequirement
  {
    std::map<std::string, std::vector<std::string>, std::less<>> values;
  };


  struct OAuthFlow
  {
    std::map<std::string, std::string, std::less<>> scopes;
    boost::urls::url authorizationUrl;
    boost::urls::url tokenUrl;
    boost::urls::url refreshUrl;
  };

  struct OAuthFlows
  {
    std::optional<OAuthFlow> implicit;
    std::optional<OAuthFlow> password;
    std::optional<OAuthFlow> clientCredentials;
    std::optional<OAuthFlow> authorizationCode;
  };

  struct SecurityScheme
  {
    boost::urls::url openIdConnectUrl;
    std::string ref;
    std::string type;
    std::string description;
    std::string name;
    std::string in;
    std::string scheme;
    std::string bearerFormat;
    std::optional<OAuthFlows> flows;
  };
}