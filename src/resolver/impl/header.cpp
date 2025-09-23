//
// Created by Rakesh on 21/09/2025.
//

#include "model/header.hpp"
#include "model/mediatype.hpp"
#include "resolver/detail.hpp"
#include "resolver/resolver.hpp"

template <>
void spt::resolver::resolve( model::Header& entity, std::string_view path )
{
  if ( entity.schema ) resolve( *entity.schema, path );
  for ( auto& [_, mt] : entity.content ) resolve( mt, path );
}
