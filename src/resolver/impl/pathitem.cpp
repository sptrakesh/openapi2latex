//
// Created by Rakesh on 21/09/2025.
//

#include "model/parameter.hpp"
#include "model/pathitem.hpp"
#include "resolver/detail.hpp"
#include "resolver/resolver.hpp"

template <>
void spt::resolver::resolve( model::PathItem& entity, std::string_view path )
{
  detail::resolve( entity, path );

  for ( auto& param : entity.parameters )
  {
    resolve( param, entity._referenceURI.empty() ? path : entity._referenceURI );
  }

  if ( entity.get )
  {
    resolve( *entity.get, entity._referenceURI.empty() ? path : entity._referenceURI );
  }

  if ( entity.post )
  {
    resolve( *entity.post, entity._referenceURI.empty() ? path : entity._referenceURI );
  }

  if ( entity.put )
  {
    resolve( *entity.put, entity._referenceURI.empty() ? path : entity._referenceURI );
  }

  if ( entity.patch )
  {
    resolve( *entity.patch, entity._referenceURI.empty() ? path : entity._referenceURI );
  }

  if ( entity._delete )
  {
    resolve( *entity._delete, entity._referenceURI.empty() ? path : entity._referenceURI );
  }
}
