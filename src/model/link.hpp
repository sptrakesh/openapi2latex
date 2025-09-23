//
// Created by Rakesh on 19/09/2025.
//

#pragma once

#include "server.hpp"

#include <any>

namespace spt::model
{
  struct Link
  {
    std::map<std::string, std::any, std::less<>> parameters;
    std::optional<Server> server;
    std::any requestBody;
    std::string operationRef;
    std::string operationId;
    std::string description;
  };
}