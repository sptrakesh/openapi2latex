//
// Created by Rakesh on 26/09/2025.
//

#pragma once

#include "model/parameter.hpp"
#include "model/requestbody.hpp"

namespace spt::output::impl
{
  struct Collectors
  {
    static Collectors& instance()
    {
      static Collectors c;
      return c;
    }

    std::map<std::string, std::reference_wrapper<const model::Parameter>, std::less<>> parameterMap{};
    std::map<std::string, std::reference_wrapper<const model::RequestBody>, std::less<>> requestBodyMap{};

  private:
    Collectors() = default;
  };
}