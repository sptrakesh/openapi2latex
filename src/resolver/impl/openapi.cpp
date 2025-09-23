//
// Created by Rakesh on 21/09/2025.
//

#include "model/openapi.hpp"
#include "resolver/detail.hpp"
#include "resolver/resolver.hpp"

template <>
void spt::resolver::resolve( model::OpenAPI& entity, std::string_view path )
{
  for ( auto& [_, pi] : entity.paths ) resolve( pi, path );
  if ( entity.components ) resolve( *entity.components, path );
}
