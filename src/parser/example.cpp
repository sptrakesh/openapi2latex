//
// Created by Rakesh on 19/09/2025.
//

#include "parser.hpp"
#include "model/example.hpp"

template <>
void spt::parser::parse( model::Example& m, c4::yml::ConstNodeRef node )
{
  for ( const auto& child : node.children() )
  {
    if ( child.key() == "summary" ) child >> m.summary;
    if ( child.key() == "description" ) child >> m.description;
    if ( child.key() == "externalValue" ) m.externalValue = parse<boost::urls::url>( child );
    if ( child.key() == "value" && child.has_val() ) m.value = std::string{ child.val().begin(), child.val().end() };
  }
}
