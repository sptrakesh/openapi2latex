//
// Created by Rakesh on 19/09/2025.
//

#include "parser.hpp"
#include "model/openapi.hpp"

template <>
void spt::parser::parse( model::Components& m, c4::yml::ConstNodeRef node )
{
  for ( const auto& child : node.children() )
  {
    if ( child.key() == "schemas" )
    {
      for ( const auto& cn : child.children() )
      {
        m.schemas.try_emplace( std::string{ std::string_view{ cn.key() } }, parse<model::Schema>( cn ) );
      }
    }

    if ( child.key() == "responses" )
    {
      for ( const auto& cn : child.children() )
      {
        m.responses.try_emplace( std::string{ std::string_view{ cn.key() } }, parse<model::Response>( cn ) );
      }
    }

    if ( child.key() == "parameters" )
    {
      for ( const auto& cn : child.children() )
      {
        m.parameters.try_emplace( std::string{ std::string_view{ cn.key() } }, parse<model::Parameter>( cn ) );
      }
    }

    if ( child.key() == "examples" )
    {
      for ( const auto& cn : child.children() )
      {
        m.examples.try_emplace( std::string{ std::string_view{ cn.key() } }, parse<model::Example>( cn ) );
      }
    }

    if ( child.key() == "requestBodies" )
    {
      for ( const auto& cn : child.children() )
      {
        m.requestBodies.try_emplace( std::string{ std::string_view{ cn.key() } }, parse<model::RequestBody>( cn ) );
      }
    }

    if ( child.key() == "headers" )
    {
      for ( const auto& cn : child.children() )
      {
        m.headers.try_emplace( std::string{ std::string_view{ cn.key() } }, parse<model::Header>( cn ) );
      }
    }

    if ( child.key() == "securitySchemes" )
    {
      for ( const auto& cn : child.children() )
      {
        m.securitySchemes.try_emplace( std::string{ std::string_view{ cn.key() } }, parse<model::SecurityScheme>( cn ) );
      }
    }
  }
}

template <>
void spt::parser::parse( model::OpenAPI& m, c4::yml::ConstNodeRef node )
{
  for ( const auto& child : node.children() )
  {
    if ( child.key() == "openapi" ) child >> m.openapi;
    if ( child.key() == "jsonSchemaDialect" ) child >> m.jsonSchemaDialect;
    if ( child.key() == "info" ) m.info = parse<model::Info>( child );
    if ( child.key() == "externalDocs" ) m.externalDocs = parse<model::ExternalDocumentation>( child );
    if ( child.key() == "components" ) m.components = parse<model::Components>( child );

    if ( child.key() == "paths" )
    {
      for ( const auto& p : child.children() )
      {
        m.paths.try_emplace( std::string{ std::string_view{ p.key() } }, parse<model::PathItem>( p ) );
      }
    }

    if ( child.key() == "webhooks" )
    {
      for ( const auto& p : child.children() )
      {
        m.webhooks.try_emplace( std::string{ std::string_view{ p.key() } }, parse<model::PathItem>( p ) );
      }
    }

    if ( child.key() == "security" )
    {
      m.security.reserve( 8 );
      for ( const auto& sec : child.children() )
      {
        m.security.emplace_back( parse<model::SecurityRequirement>( sec ) );
      }
    }

    if ( child.key() == "servers" )
    {
      m.servers.reserve( 8 );
      for ( const auto& srv : child.children() )
      {
        m.servers.emplace_back( parse<model::Server>( srv ) );
      }
    }

    if ( child.key() == "tags" )
    {
      m.tags.reserve( 8 );
      for ( const auto& tag : child.children() )
      {
        m.tags.emplace_back( parse<model::Tag>( tag ) );
      }
    }

    if ( child.key() == "x-tagGroups" )
    {
      m.tagGroups.reserve( 8 );
      for ( const auto& tag : child.children() )
      {
        m.tagGroups.emplace_back( parse<model::TagGroup>( tag ) );
      }
    }
  }
}
