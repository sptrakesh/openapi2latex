//
// Created by Rakesh on 19/09/2025.
//

#include "parser.hpp"
#include "model/parameter.hpp"

template <>
void spt::parser::parse( model::Parameter& m, c4::yml::ConstNodeRef node )
{
  for ( const auto& child : node.children() )
  {
    if ( child.key() == "$ref" ) child >> m.ref;
    if ( child.key() == "name" ) child >> m.name;
    if ( child.key() == "in" ) child >> m.in;
    if ( child.key() == "description" ) child >> m.description;
    if ( child.key() == "style" ) child >> m.style;
    if ( child.key() == "required" ) child >> m.required;
    if ( child.key() == "deprecated" ) child >> m.deprecated;
    if ( child.key() == "allowEmptyValue" ) child >> m.allowEmptyValue;
    if ( child.key() == "explode" ) child >> m.explode;
    if ( child.key() == "allowReserved" ) child >> m.allowReserved;
    if ( child.key() == "example" && child.has_val() ) m.example = std::string{ child.val().begin(), child.val().end() };
    if ( child.key() == "examples" )
    {
      for ( const auto& ex : child.children() )
      {
        auto key = std::string_view{ ex.key() };
        m.examples.try_emplace( std::string{ key }, parse<model::Example>( ex ) );
      }
    }
    if ( child.key() == "schema" ) m.schema = parse<model::Schema>( child );
  }
}
