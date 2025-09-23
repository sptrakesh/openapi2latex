//
// Created by Rakesh on 19/09/2025.
//

#include "parser.hpp"
#include "model/link.hpp"

template <>
void spt::parser::parse( model::Link& m, c4::yml::ConstNodeRef node )
{
  for ( const auto& child : node.children() )
  {
    if ( child.key() == "operationRef" ) child >> m.operationRef;
    if ( child.key() == "operationId" ) child >> m.operationId;
    if ( child.key() == "description" ) child >> m.description;
    if ( child.key() == "requestBody" && child.has_val() ) m.requestBody = std::string{ child.val().begin(), child.val().end() };
    if ( child.key() == "server" ) m.server = parse<model::Server>( child );
    
    if ( child.key() == "parameters" )
    {
      for ( const auto& p : child.children() )
      {
        if ( !p.has_val() ) continue;
        auto key = std::string_view{ p.key() };
        m.parameters.try_emplace( std::string{ key }, std::string{ p.val().begin(), p.val().end() } );
      }
    }
  }
}
