//
// Created by Rakesh on 21/09/2025.
//

#include "model/parameter.hpp"
#include "resolver/detail.hpp"
#include "resolver/resolver.hpp"

template <>
void spt::resolver::resolve( model::Parameter& entity, std::string_view path )
{
  detail::resolve( entity, path );
}
