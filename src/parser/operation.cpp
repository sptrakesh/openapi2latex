//
// Created by Rakesh on 19/09/2025.
//

#include "parser.hpp"
#include "model/operation.hpp"
#include "model/parameter.hpp"
#include "model/response.hpp"
#include "model/pathitem.hpp"

template <>
void spt::parser::parse( model::Operation& m, c4::yml::ConstNodeRef node )
{
  for ( const auto& child : node.children() )
  {
    if ( child.key() == "summary" ) child >> m.summary;
    if ( child.key() == "description" ) child >> m.description;
    if ( child.key() == "operationId" ) child >> m.operationId;
    if ( child.key() == "x-since-version" ) child >> m.sinceVersion;
    if ( child.key() == "deprecated" ) child >> m.deprecated;
    if ( child.key() == "externalDocs" ) m.externalDocs = parse<model::ExternalDocumentation>( child );
    if ( child.key() == "requestBody" ) m.requestBody = parse<model::RequestBody>( child );

    if ( child.key() == "security" )
    {
      m.security.reserve( 8 );
      for ( const auto& sec : child.children() ) m.security.emplace_back( parse<model::SecurityRequirement>( sec ) );
    }

    if ( child.key() == "servers" )
    {
      m.servers.reserve( 8 );
      for ( const auto& srv : child.children() ) m.servers.emplace_back( parse<model::Server>( srv ) );
    }

    if ( child.key() == "parameters" )
    {
      m.parameters.reserve( 8 );
      for ( const auto& parm : child.children() ) m.parameters.emplace_back( parse<model::Parameter>( parm ) );
    }

    if ( child.key() == "tags" )
    {
      m.tags.reserve( 8 );
      for ( const auto& tag : child.children() )
      {
        std::string t;
        tag >> t;
        m.tags.push_back( std::move( t ) );
      }
    }

    if ( child.key() == "x-codeSamples" )
    {
      m.codeSamples.reserve( 8 );
      for ( const auto& sam : child.children() ) m.codeSamples.emplace_back( parse<model::CodeSample>( sam ) );
    }

    if ( child.key() == "responses" )
    {
      for ( const auto& resp : child.children() )
      {
        m.responses.try_emplace( std::string{ std::string_view{ resp.key() } }, parse<model::Response>( resp ) );
      }
    }

    if ( child.key() == "callbacks" )
    {
      for ( const auto& cb : child.children() )
      {
        m.callbacks.try_emplace( std::string{ std::string_view{ cb.key() } }, parse<model::PathItem>( cb ) );
      }
    }
  }
}
