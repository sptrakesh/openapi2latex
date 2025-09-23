//
// Created by Rakesh on 21/09/2025.
//

#include "model/mediatype.hpp"
#include "resolver/detail.hpp"
#include "resolver/resolver.hpp"

template <>
void spt::resolver::resolve( model::MediaType& entity, std::string_view path )
{
  if ( entity.schema ) resolve( *entity.schema, path );
  for ( auto& [_, enc] : entity.encoding )
  {
    for ( auto& [_i, head] : enc.headers ) resolve( head, path );
  }
}
