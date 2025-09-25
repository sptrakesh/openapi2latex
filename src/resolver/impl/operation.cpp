//
// Created by Rakesh on 23/09/2025.
//

#include "model/operation.hpp"
#include "model/parameter.hpp"
#include "model/pathitem.hpp"
#include "resolver/detail.hpp"
#include "resolver/resolver.hpp"

template <>
void spt::resolver::resolve( model::Operation& entity, std::string_view path )
{
  for ( auto& param : entity.parameters ) resolve( param, path );
  for ( auto& [_, resp] : entity.responses ) resolve( resp, path );
  for ( auto& [_, cb] : entity.callbacks ) resolve( cb, path );
  if ( entity.requestBody ) resolve( *entity.requestBody, path );
}
