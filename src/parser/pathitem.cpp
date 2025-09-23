//
// Created by Rakesh on 19/09/2025.
//

#include "parser.hpp"
#include "model/operation.hpp"
#include "model/parameter.hpp"
#include "model/pathitem.hpp"

template <>
void spt::parser::parse( model::PathItem& m, c4::yml::ConstNodeRef node )
{
  for ( const auto& child : node.children() )
  {
    if ( child.key() == "$ref" ) child >> m.ref;
    if ( child.key() == "summary" ) child >> m.summary;
    if ( child.key() == "description" ) child >> m.description;
    if ( child.key() == "get" ) m.get = parse<model::Operation>( child );
    if ( child.key() == "put" ) m.put = parse<model::Operation>( child );
    if ( child.key() == "post" ) m.post = parse<model::Operation>( child );
    if ( child.key() == "delete" ) m._delete = parse<model::Operation>( child );
    if ( child.key() == "options" ) m.options = parse<model::Operation>( child );
    if ( child.key() == "head" ) m.head = parse<model::Operation>( child );
    if ( child.key() == "patch" ) m.patch = parse<model::Operation>( child );
    if ( child.key() == "trace" ) m.trace = parse<model::Operation>( child );

    if ( child.key() == "servers" )
    {
      m.servers.reserve( 8 );
      for ( const auto& server : child.children() ) m.servers.emplace_back( parse<model::Server>( server ) );
    }

    if ( child.key() == "parameters" )
    {
      m.parameters.reserve( 8 );
      for ( const auto& param : child.children() ) m.parameters.emplace_back( parse<model::Parameter>( param ) );
    }
  }
}
