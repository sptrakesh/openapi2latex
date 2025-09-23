//
// Created by Rakesh on 21/09/2025.
//

#include "model/openapi.hpp"
#include "resolver/detail.hpp"
#include "resolver/resolver.hpp"

template <>
void spt::resolver::resolve( model::Components& entity, std::string_view path )
{
  for ( auto& [_, schema] : entity.schemas ) resolve( schema, path );
  for ( auto& [_, head] : entity.headers ) resolve( head, path );
  for ( auto& [_, resp] : entity.responses ) resolve( resp, path );
  for ( auto& [_, parm] : entity.parameters ) resolve( parm, path );
}
