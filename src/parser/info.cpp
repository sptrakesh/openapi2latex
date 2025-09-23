//
// Created by Rakesh on 19/09/2025.
//

#include "parser.hpp"
#include "model/info.hpp"

template <>
void spt::parser::parse( model::Info::Contact& m, c4::yml::ConstNodeRef node )
{
  for ( const auto& child : node.children() )
  {
    if ( child.key() == "name" ) child >> m.name;
    if ( child.key() == "email" ) child >> m.email;
    if ( child.key() == "url" ) m.url = parse<boost::urls::url>( child );
  }
}

template <>
void spt::parser::parse( model::Info::License& m, c4::yml::ConstNodeRef node )
{
  for ( const auto& child : node.children() )
  {
    if ( child.key() == "name" ) child >> m.name;
    if ( child.key() == "identifier" ) child >> m.identifier;
    if ( child.key() == "url" ) m.url = parse<boost::urls::url>( child );
  }
}

template <>
void spt::parser::parse( model::Info& m, c4::yml::ConstNodeRef node )
{
  for ( const auto& child : node.children() )
  {
    if ( child.key() == "title" ) child >> m.title;
    if ( child.key() == "summary" ) child >> m.summary;
    if ( child.key() == "description" ) child >> m.description;
    if ( child.key() == "version" ) child >> m.version;
    if ( child.key() == "termsOfService" ) m.termsOfService = parse<boost::urls::url>( child );
    if ( child.key() == "license" ) m.license = parse<model::Info::License>( child );
    if ( child.key() == "contact" ) m.contact = parse<model::Info::Contact>( child );
  }
}
