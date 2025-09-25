//
// Created by Rakesh on 21/09/2025.
//

#pragma once

#include "concept.hpp"
#include "filecache.hpp"
#include "log/NanoLog.hpp"
#include "parser/parser.hpp"
#include "util/split.hpp"

namespace spt::resolver::detail
{
  template <HasRef T>
  void resolve( T& entity, std::string_view filePath )
  {
    if ( filePath.empty() || entity.ref.empty() ) return;

    auto cleanedPath = std::string{ filePath };
    if ( auto pos = cleanedPath.find( '#' ); pos != std::string::npos )
    {
      cleanedPath = cleanedPath.substr( 0, pos );
    }

    std::string fn;
    const auto it = entity.ref.find( '#' );

    if ( entity.ref.front() == '#' ) fn = cleanedPath;
    else
    {
      auto p = std::filesystem::path( cleanedPath ).parent_path();

      if ( it == std::string::npos )
      {
        fn = std::filesystem::weakly_canonical( p.append( entity.ref ) ).make_preferred().string();
      }
      else
      {
        fn = std::filesystem::weakly_canonical( p.append( entity.ref.substr( 0, it ) ) ).make_preferred().string();
      }
    }

    if ( !std::filesystem::exists( fn ) )
    {
      LOG_WARN << "Reference file not found: " << fn;
      return;
    }

    entity._referenceURI = it == std::string::npos ? fn : std::format( "{}{}", fn, entity.ref.substr( it ) );
    LOG_DEBUG << "Reference URI: " << entity._referenceURI;

    std::string refPath = it == std::string::npos ? "" : entity.ref.substr( it + 1 );
    auto parts = util::split( refPath, 4, "/" );
    LOG_DEBUG << "Reference path: " << refPath << " parts: " << std::format( "{:n}", parts );

    const auto expected = FileCache::instance().yaml( fn );
    if ( !expected.has_value() )
    {
      LOG_WARN << expected.error();
      return;
    }

    const auto root = expected.value();
    auto node = root;

    if ( it == std::string::npos )
    {
      parser::parse( entity, root );
      return;
    }

    std::size_t idx = 0;
    for ( auto ref : parts )
    {
      for ( const auto& child : node.children() )
      {
        if ( child.key() == ref && child.is_map() )
        {
          if ( idx == parts.size() - 1 ) parser::parse( entity, child );
          node = child;
          break;
        }
      }

      ++idx;
    }
  }
}