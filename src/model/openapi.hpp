//
// Created by Rakesh on 18/09/2025.
//

#pragma once

#include "example.hpp"
#include "header.hpp"
#include "info.hpp"
#include "parameter.hpp"
#include "pathitem.hpp"
#include "requestbody.hpp"
#include "response.hpp"
#include "security.hpp"
#include "tag.hpp"

namespace spt::model
{
  struct Components
  {
    std::map<std::string, Schema, std::less<>> schemas;
    std::map<std::string, Response, std::less<>> responses;
    std::map<std::string, Parameter, std::less<>> parameters;
    std::map<std::string, Example, std::less<>> examples;
    std::map<std::string, RequestBody, std::less<>> requestBodies;
    std::map<std::string, Header, std::less<>> headers;
    std::map<std::string, SecurityScheme, std::less<>> securitySchemes;
  };

  struct OpenAPI
  {
    std::map<std::string, PathItem, std::less<>> paths;
    std::map<std::string, PathItem, std::less<>> webhooks;
    std::vector<Server> servers;
    std::vector<SecurityRequirement> security;
    std::vector<Tag> tags;
    std::vector<TagGroup> tagGroups;
    Info info;
    std::optional<ExternalDocumentation> externalDocs;
    std::optional<Components> components;
    std::string openapi;
    std::string jsonSchemaDialect;
    std::string _referenceURI;
  };
}