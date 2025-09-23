//
// Created by Rakesh on 19/09/2025.
//

#include "parser.hpp"
#include "model/server.hpp"

template <>
void spt::parser::parse( model::Server::ServerVariable& m, c4::yml::ConstNodeRef node )
{
  for ( const auto& child : node.children() )
  {
    if ( child.key() == "description" ) child >> m.description;
    if ( child.key() == "default" ) child >> m._default;
    if ( child.key() == "enum" )
    {
      m._enum.reserve( 8 );
      for ( const auto& e : child.children() )
      {
        auto& v = m._enum.emplace_back();
        e >> v;
      }
    }
  }
}

template <>
void spt::parser::parse( model::Server& m, c4::yml::ConstNodeRef node )
{
  for ( const auto& child : node.children() )
  {
    if ( child.key() == "description" ) child >> m.description;
    if ( child.key() == "url" ) m.url = parse<boost::urls::url>( child );
    if ( child.key() == "variables" )
    {
      for ( const auto& v : child.children() )
      {
        auto key = std::string_view{ v.key() };
        m.variables.try_emplace( std::string{ key }, parse<model::Server::ServerVariable>( v ) );
      }
    }
  }
}
