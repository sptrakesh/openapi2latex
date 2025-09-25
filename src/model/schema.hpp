//
// Created by Rakesh on 19/09/2025.
//

#pragma once

#include "external.hpp"

#include <any>
#include <map>
#include <memory>
#include <optional>
#include <vector>

namespace spt::model
{
  struct ExternalDocumentation;

  struct Discriminator
  {
    std::map<std::string, std::string, std::less<>> mapping;
    std::string propertyName;
  };

  struct XML
  {
    std::string name;
    std::string _namespace;
    std::string prefix;
    bool attribute = false;
    bool wrapped = false;
  };

  struct Schema
  {
    std::optional<Discriminator> discriminator;
    std::optional<XML> xml;
    std::optional<ExternalDocumentation> externalDocs;
    std::map<std::string, Schema, std::less<>> properties;
    std::unique_ptr<Schema> items;
    std::vector<Schema> allOf;
    std::vector<Schema> oneOf;
    std::vector<Schema> anyOf;
    std::vector<std::string> required;
    std::vector<std::string> enumeration;
    std::vector<std::any> examples;
    std::any example;
    std::any _default;
    std::string _referenceURI;
    std::string ref;
    std::string type;
    std::string title;
    std::string summary;
    std::string description;
    std::string pattern;
    std::string format;
    std::string dialect;
    std::string sinceVersion;
    std::optional<double> maximum;
    std::optional<double> exclusiveMaximum;
    std::optional<double> minimum;
    std::optional<double> exclusiveMinimum;
    std::optional<double> maxLength;
    std::optional<double> minLength;
    std::optional<double> maxItems;
    std::optional<double> minItems;
    bool nullable = false;
    bool readOnly = false;
    bool writeOnly = false;
    bool deprecated = false;
  };
}