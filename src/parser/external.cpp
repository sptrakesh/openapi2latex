//
// Created by Rakesh on 19/09/2025.
//

#include "parser.hpp"
#include "model/external.hpp"

template <>
void spt::parser::parse( model::ExternalDocumentation& m, c4::yml::ConstNodeRef node )
{
  for ( const auto& child : node.children() )
  {
    if ( child.key() == "description" ) child >> m.description;
    if ( child.key() == "url" ) m.url = parse<boost::urls::url>( child );
  }
}