//
// Created by Rakesh on 21/09/2025.
//

#include "model/response.hpp"
#include "resolver/detail.hpp"
#include "resolver/resolver.hpp"

template <>
void spt::resolver::resolve( model::Response& entity, std::string_view path )
{
  for ( auto& [_, head] : entity.headers ) resolve( head, path );
  for ( auto& [_, mt] : entity.content ) resolve( mt, path );
}
