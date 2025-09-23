//
// Created by Rakesh on 19/09/2025.
//

#include "parser.hpp"
#include "model/security.hpp"

#include <boost/url/parse.hpp>

template <>
void spt::parser::parse( model::SecurityRequirement& m, c4::yml::ConstNodeRef node )
{
  for ( const auto& value : node.children() )
  {
    auto vec = std::vector<std::string>{};
    vec.reserve( 8 );
    for ( const auto& v : value.children() )
    {
      auto& sv = vec.emplace_back();
      v >> sv;
    }
    auto key = std::string_view{ value.key() };
    m.values.try_emplace( std::string{ key }, std::move( vec ) );
  }
}

template <>
void spt::parser::parse( model::OAuthFlow& m, c4::yml::ConstNodeRef node )
{
  for ( const auto& child : node.children() )
  {
    if ( child.key() == "authorizationUrl" ) m.authorizationUrl = parse<boost::urls::url>( child );
    if ( child.key() == "tokenUrl" ) m.tokenUrl = parse<boost::urls::url>( child );
    if ( child.key() == "refreshUrl" ) m.refreshUrl = parse<boost::urls::url>( child );
    if ( child.key() == "scopes" )
    {
      for ( const auto& v : child.children() )
      {
        auto value = std::string{};
        v >> value;
        auto key = std::string_view{ v.key() };
        m.scopes.try_emplace( std::string{ key }, std::move( value ) );
      }
    }
  }
}

template <>
void spt::parser::parse( model::OAuthFlows& m, c4::yml::ConstNodeRef node )
{
  for ( const auto& child : node.children() )
  {
    if ( child.key() == "implicit" ) m.implicit = parse<model::OAuthFlow>( child );
    if ( child.key() == "password" ) m.password = parse<model::OAuthFlow>( child );
    if ( child.key() == "clientCredentials" ) m.clientCredentials = parse<model::OAuthFlow>( child );
    if ( child.key() == "authorizationCode" ) m.authorizationCode = parse<model::OAuthFlow>( child );
  }
}

template <>
void spt::parser::parse( model::SecurityScheme& m, c4::yml::ConstNodeRef node )
{
  for ( const auto& child : node.children() )
  {
    if ( child.key() == "openIdConnectUrl" ) m.openIdConnectUrl = parse<boost::urls::url>( child );
    if ( child.key() == "$ref" ) child >> m.ref;
    if ( child.key() == "type" ) child >> m.type;
    if ( child.key() == "description" ) child >> m.description;
    if ( child.key() == "name" ) child >> m.name;
    if ( child.key() == "in" ) child >> m.in;
    if ( child.key() == "scheme" ) child >> m.scheme;
    if ( child.key() == "bearerFormat" ) child >> m.bearerFormat;
    if ( child.key() == "flows" ) m.flows = parse<model::OAuthFlows>( child );
  }
}
