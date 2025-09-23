//
// Created by Rakesh on 21/09/2025.
//

#include "model/pathitem.hpp"
#include "resolver/detail.hpp"
#include "resolver/resolver.hpp"

template <>
void spt::resolver::resolve( model::PathItem& entity, std::string_view path )
{
  detail::resolve( entity, path );
}
