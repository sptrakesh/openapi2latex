//
// Created by Rakesh on 19/09/2025.
//

#include "parser.hpp"
#include "model/schema.hpp"

#include "model/example.hpp"

template <>
spt::model::Discriminator spt::parser::parse( c4::yml::ConstNodeRef node )
{
  auto m = model::Discriminator{};

  for ( const auto& child : node.children() )
  {
    if ( child.key() == "propertyName" ) child >> m.propertyName;
    if ( child.key() == "mapping" )
    {
      for ( const auto& map : child.children() )
      {
        if ( !map.has_val() ) continue;
        m.mapping.try_emplace( std::string{ std::string_view{ map.key() } }, std::string{ map.val().begin(), map.val().end() } );
      }
    }
  }

  return m;
}

template <>
spt::model::XML spt::parser::parse( c4::yml::ConstNodeRef node )
{
  auto m = model::XML{};

  for ( const auto& child : node.children() )
  {
    if ( child.key() == "name" ) child >> m.name;
    if ( child.key() == "namespace" ) child >> m._namespace;
    if ( child.key() == "prefix" ) child >> m.prefix;
    if ( child.key() == "attribute" ) child >> m.attribute;
    if ( child.key() == "wrapped" ) child >> m.wrapped;
  }

  return m;
}

namespace
{
  namespace pparser
  {
    using namespace spt;

    void parse( model::Schema& m, c4::yml::ConstNodeRef node )
    {
      for ( const auto& child : node.children() )
      {
        if ( child.key() == "$ref" ) child >> m.ref;
        if ( child.key() == "type" ) child >> m.type;
        if ( child.key() == "title" ) child >> m.title;
        if ( child.key() == "summary" ) child >> m.summary;
        if ( child.key() == "description" ) child >> m.description;
        if ( child.key() == "pattern" ) child >> m.pattern;
        if ( child.key() == "format" ) child >> m.format;
        if ( child.key() == "dialect" ) child >> m.dialect;
        if ( child.key() == "x-since-version" ) child >> m.sinceVersion;
        if ( child.key() == "nullable" ) child >> m.nullable;
        if ( child.key() == "readOnly" ) child >> m.readOnly;
        if ( child.key() == "writeOnly" ) child >> m.writeOnly;
        if ( child.key() == "deprecated" ) child >> m.deprecated;
        if ( child.key() == "discriminator" ) m.discriminator = parser::parse<model::Discriminator>( child );
        if ( child.key() == "xml" ) m.xml = parser::parse<model::XML>( child );
        if ( child.key() == "externalDocs" ) m.externalDocs = parser::parse<model::ExternalDocumentation>( child );
        if ( child.key() == "default" && child.has_val() ) m._default = std::string{ child.val().begin(), child.val().end() };
        if ( child.key() == "example" && child.has_val() ) m.example = std::string{ child.val().begin(), child.val().end() };
        if ( child.key() == "examples" )
        {
          for ( const auto& ex : child.children() )
          {
            if ( !ex.has_val() ) continue;
            m.examples.emplace_back( std::string{ ex.val().begin(), ex.val().end() } );
          }
        }

        if ( child.key() == "maximum" )
        {
          m.maximum.emplace();
          child >> *m.maximum;
        }

        if ( child.key() == "exclusiveMaximum" )
        {
          m.exclusiveMaximum.emplace();
          child >> *m.exclusiveMaximum;
        }

        if ( child.key() == "minimum" )
        {
          m.minimum.emplace();
          child >> *m.minimum;
        }

        if ( child.key() == "exclusiveMinimum" )
        {
          m.exclusiveMinimum.emplace();
          child >> *m.exclusiveMinimum;
        }

        if ( child.key() == "maxLength" )
        {
          m.maxLength.emplace();
          child >> *m.maxLength;
        }

        if ( child.key() == "minLength" )
        {
          m.minLength.emplace();
          child >> *m.minLength;
        }

        if ( child.key() == "maxItems" )
        {
          m.maxItems.emplace();
          child >> *m.maxItems;
        }

        if ( child.key() == "minItems" )
        {
          m.minItems.emplace();
          child >> *m.minItems;
        }

        if ( child.key() == "properties" )
        {
          for ( const auto& p : child.children() )
          {
            auto s = model::Schema{};
            parse( s, p );
            m.properties.try_emplace( std::string{ std::string_view{ p.key() } }, std::move( s ) );
          }
        }

        if ( child.key() == "items" )
        {
          m.items = std::make_unique<model::Schema>();
          parse( *m.items, child );
        }

        if ( child.key() == "allOf" )
        {
          m.allOf.reserve( 4 );
          for ( const auto& p : child.children() )
          {
            if ( p.is_map() ) m.allOf.emplace_back( spt::parser::parse<model::Schema>( p ) );
          }
        }

        if ( child.key() == "oneOf" )
        {
          m.oneOf.reserve( 4 );
          for ( const auto& p : child.children() )
          {
            if ( p.is_map() ) m.oneOf.emplace_back( spt::parser::parse<model::Schema>( p ) );
          }
        }

        if ( child.key() == "anyOf" )
        {
          m.anyOf.reserve( 4 );
          for ( const auto& p : child.children() )
          {
            if ( p.is_map() ) m.anyOf.emplace_back( spt::parser::parse<model::Schema>( p ) );
          }
        }
      }
    }
  }
}

template <>
void spt::parser::parse( model::Schema& m, c4::yml::ConstNodeRef node )
{
  pparser::parse( m, node );
}
