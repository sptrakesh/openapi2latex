//
// Created by Rakesh on 18/09/2025.
//

#pragma once

#include "codesample.hpp"
#include "external.hpp"
#include "requestbody.hpp"
#include "response.hpp"
#include "security.hpp"
#include "server.hpp"

namespace spt::model
{
  struct Parameter;
  struct PathItem;

  struct Operation
  {
    std::vector<Server> servers;
    std::vector<Parameter> parameters;
    std::vector<std::string> tags;
    std::vector<CodeSample> codeSamples;
    std::map<std::string, Response, std::less<>> responses;
    std::map<std::string, PathItem, std::less<>> callbacks;
    std::optional<ExternalDocumentation> externalDocs;
    std::optional<RequestBody> requestBody;
    std::vector<SecurityRequirement> security;
    std::string summary;
    std::string description;
    std::string operationId;
    std::string sinceVersion;
    bool deprecated;
  };
}