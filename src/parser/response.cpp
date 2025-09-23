//
// Created by Rakesh on 19/09/2025.
//

#include "parser.hpp"
#include "model/response.hpp"

template <>
void spt::parser::parse( model::Response& m, c4::yml::ConstNodeRef node )
{
  for ( const auto& child : node.children() )
  {
    if ( child.key() == "$ref" ) child >> m.ref;
    if ( child.key() == "description" ) child >> m.description;

    if ( child.key() == "headers" )
    {
      for ( const auto& h : child.children() )
      {
        m.headers.try_emplace( std::string{ std::string_view{ h.key() } }, parse<model::Header>( h ) );
      }
    }

    if ( child.key() == "content" )
    {
      for ( const auto& h : child.children() )
      {
        m.content.try_emplace( std::string{ std::string_view{ h.key() } }, parse<model::MediaType>( h ) );
      }
    }

    if ( child.key() == "links" )
    {
      for ( const auto& h : child.children() )
      {
        m.links.try_emplace( std::string{ std::string_view{ h.key() } }, parse<model::Link>( h ) );
      }
    }
  }
}
