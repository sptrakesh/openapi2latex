//
// Created by Rakesh on 19/09/2025.
//

#include "parser.hpp"
#include "model/codesample.hpp"

template <>
void spt::parser::parse( model::CodeSample& m, c4::yml::ConstNodeRef node )
{
  for ( const auto& child : node.children() )
  {
    if ( child.key() == "lang" ) child >> m.lang;
    if ( child.key() == "label" ) child >> m.label;
    if ( child.key() == "source" ) child >> m.source;
  }
}
