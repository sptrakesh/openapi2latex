//
// Created by Rakesh on 23/09/2025.
//

#include "model/requestbody.hpp"
#include "resolver/detail.hpp"
#include "resolver/resolver.hpp"

template <>
void spt::resolver::resolve( model::RequestBody& entity, std::string_view path )
{
  detail::resolve( entity, path );
  for ( auto& [_, mt] : entity.content ) resolve( mt, entity._referenceURI.empty() ? path : entity._referenceURI );
}
