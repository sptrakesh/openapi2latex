//
// Created by Rakesh on 19/09/2025.
//

#include "parser.hpp"
#include "model/tag.hpp"

template <>
void spt::parser::parse( model::Tag& m, c4::yml::ConstNodeRef node )
{
  for ( const auto& child : node.children() )
  {
    if ( child.key() == "name" ) child >> m.name;
    if ( child.key() == "description" ) child >> m.description;
    if ( child.key() == "externalDocs" ) m.externalDocs = parse<model::ExternalDocumentation>( child );
  }
}

template <>
void spt::parser::parse( model::TagGroup& m, c4::yml::ConstNodeRef node )
{
  for ( const auto& child : node.children() )
  {
    if ( child.key() == "name" ) child >> m.name;
    if ( child.key() == "tags" )
    {
      m.tags.reserve( 8 );
      for ( const auto& tag : child.children() )
      {
        auto& t = m.tags.emplace_back();
        tag >> t;
      }
    }
  }
}
