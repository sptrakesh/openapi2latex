//
// Created by Rakesh on 19/09/2025.
//

#include "parser.hpp"
#include "model/example.hpp"
#include "model/header.hpp"
#include "model/mediatype.hpp"

template <>
void spt::parser::parse( model::Header& m, c4::yml::ConstNodeRef node )
{
  static const auto prefix = std::string( "example: " );
  for ( const auto& child : node.children() )
  {
    if ( child.key() == "description" ) child >> m.description;
    if ( child.key() == "style" ) child >> m.style;
    if ( child.key() == "required" ) child >> m.required;
    if ( child.key() == "deprecated" ) child >> m.deprecated;
    if ( child.key() == "allowEmptyValue" ) child >> m.allowEmptyValue;
    if ( child.key() == "explode" ) child >> m.explode;

    if ( child.key() == "example" )
    {
      if ( child.has_val() ) m.example = std::string{ child.val().begin(), child.val().end() };
      else if ( child.is_seq() || child.is_map() )
      {
        auto v = ryml::emitrs_yaml<std::string>( child );
        m.example = v.starts_with( prefix ) ? v.substr( prefix.size() ) : std::move( v );
      }
    }

    if ( child.key() == "examples" )
    {
      auto key = std::string_view{ child.key() };
      m.examples.try_emplace( std::string{ key }, parse<model::Example>( child ) );
    }

    if ( child.key() == "content" )
    {
      auto key = std::string_view{ child.key() };
      m.content.try_emplace( std::string{ key }, parse<model::MediaType>( child ) );
    }

    if ( child.key() == "content" ) m.schema = parse<model::Schema>( child );
  }
}
