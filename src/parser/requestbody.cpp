//
// Created by Rakesh on 19/09/2025.
//

#include "parser.hpp"
#include "model/requestbody.hpp"

template <>
void spt::parser::parse( model::RequestBody& m, c4::yml::ConstNodeRef node )
{
  for ( const auto& child : node.children() )
  {
    if ( child.key() == "$ref" ) child >> m.ref;
    if ( child.key() == "description" ) child >> m.description;
    if ( child.key() == "required" ) child >> m.required;

    if ( child.key() == "content" )
    {
      for ( const auto& c : child.children() )
      {
        m.content.try_emplace( std::string{ std::string_view{ c.key() } }, parse<model::MediaType>( c ) );
      }
    }
  }
}
