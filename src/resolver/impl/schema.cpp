//
// Created by Rakesh on 21/09/2025.
//

#include "model/schema.hpp"
#include "resolver/detail.hpp"
#include "resolver/resolver.hpp"

template <>
void spt::resolver::resolve( model::Schema& entity, std::string_view path )
{
  detail::resolve( entity, path );

  auto idx = entity._referenceURI.find( '#' );
  auto fp = idx != std::string::npos ? entity._referenceURI.substr( 0, idx ) : entity._referenceURI;
  if ( entity.items ) resolve( *entity.items, fp );
  for ( auto& child : entity.allOf ) resolve( child, fp );
  for ( auto& child : entity.oneOf ) resolve( child, fp );
  for ( auto& child : entity.anyOf ) resolve( child, fp );
}
