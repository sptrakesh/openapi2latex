//
// Created by Rakesh on 21/09/2025.
//

#pragma once

#include <string>

namespace spt::resolver
{
  template <typename T>
  concept HasRef = requires( T t )
  {
    std::is_same_v<decltype(t.ref), std::string> && std::is_same_v<decltype(t._referenceURI), std::string>;
  };
}