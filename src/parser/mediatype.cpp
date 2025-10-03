//
// Created by Rakesh on 19/09/2025.
//

#include "parser.hpp"
#include "model/header.hpp"
#include "model/mediatype.hpp"

template <>
void spt::parser::parse( model::MediaType::Encoding& m, c4::yml::ConstNodeRef node )
{
  for ( const auto& child : node.children() )
  {
    if ( child.key() == "contentType" ) child >> m.contentType;
    if ( child.key() == "style" ) child >> m.style;
    if ( child.key() == "explode" ) child >> m.explode;
    if ( child.key() == "allowReserved" ) child >> m.allowReserved;

    if ( child.key() == "headers" )
    {
      for ( const auto& ex : child.children() )
      {
        auto key = std::string_view{ ex.key() };
        m.headers.try_emplace( std::string{ key }, parse<model::Header>( ex ) );
      }
    }
  }
}

template <>
void spt::parser::parse( model::MediaType& m, c4::yml::ConstNodeRef node )
{
  for ( const auto& child : node.children() )
  {
    if ( child.key() == "example" )
    {
      if ( child.has_val() ) m.example = std::string{ child.val().begin(), child.val().end() };
      else if ( child.is_seq() || child.is_map() ) m.example = ryml::emitrs_yaml<std::string>( child );
    }
    if ( child.key() == "schema" ) m.schema = parse<model::Schema>( child );

    if ( child.key() == "examples" )
    {
      for ( const auto& ex : child.children() )
      {
        auto key = std::string_view{ ex.key() };
        m.examples.try_emplace( std::string{ key }, parse<model::Example>( ex ) );
      }
    }

    if ( child.key() == "encoding" )
    {
      for ( const auto& ex : child.children() )
      {
        auto key = std::string_view{ ex.key() };
        m.encoding.try_emplace( std::string{ key }, parse<model::MediaType::Encoding>( ex ) );
      }
    }
  }
}
