//
// Created by Rakesh on 19/09/2025.
//

#pragma once

#include <boost/url/parse.hpp>
#include <boost/url/url.hpp>
#include <ryml.hpp>
#include <ryml_std.hpp>

namespace spt::parser
{
  template <typename T>
  void parse( T& model, c4::yml::ConstNodeRef node );

  template <typename T>
  T parse( c4::yml::ConstNodeRef node )
  {
    T t;
    parse( t, node );
    return t;
  }
}

template <>
inline boost::urls::url spt::parser::parse( c4::yml::ConstNodeRef node )
{
  auto url = std::string{};
  node >> url;
  return boost::urls::parse_uri( url ).value();
}