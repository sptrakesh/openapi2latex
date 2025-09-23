//
// Created by Rakesh on 18/09/2025.
//

#pragma once

#include "operation.hpp"

namespace spt::model
{
  struct PathItem
  {
    std::vector<Server> servers;
    std::vector<Parameter> parameters;
    std::optional<Operation> get;
    std::optional<Operation> put;
    std::optional<Operation> post;
    std::optional<Operation> _delete;
    std::optional<Operation> options;
    std::optional<Operation> head;
    std::optional<Operation> patch;
    std::optional<Operation> trace;
    std::string ref;
    std::string summary;
    std::string description;
    std::string _referenceURI;
  };
}