//
// Created by Rakesh on 20/09/2025.
//

#pragma once

#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <map>

#include <ryml.hpp>

namespace spt::resolver
{
  struct FileCache
  {
    static FileCache& instance()
    {
      static FileCache instance;
      return instance;
    }

    std::expected<std::string, std::string> contents( const std::string& filePath )
    {
      using O = std::expected<std::string, std::string>;

      if ( !std::filesystem::exists( filePath ) ) return O{ std::unexpect, std::format( "File {} not found.", filePath ) };
      if ( !contentCache.contains( filePath ) ) load( filePath );
      return O{ std::in_place, contentCache.at( filePath ) };
    }

    std::expected<c4::yml::ConstNodeRef, std::string> yaml( const std::string& filePath )
    {
      using O = std::expected<c4::yml::ConstNodeRef, std::string>;

      if ( !std::filesystem::exists( filePath ) ) return O{ std::unexpect, std::format( "File {} not found.", filePath ) };
      if ( !yamlCache.contains( filePath ) )
      {
        if ( !contentCache.contains( filePath ) ) load( filePath );
        yamlCache.try_emplace( filePath, ryml::parse_in_place( contentCache.at( filePath ).data() ) );
      }

      return O{ std::in_place, yamlCache.at( filePath ).rootref() };
    }

  private:
    FileCache() = default;

    void load( const std::string& filePath )
    {
      const auto path = std::filesystem::path( filePath );
      const auto size = std::filesystem::file_size( path );
      std::ifstream f( filePath, std::ios::in | std::ios::binary );
      contentCache.try_emplace( filePath, std::string( size, '\0' ) );
      f.read( contentCache.at( filePath ).data(), static_cast<std::streamsize>( size ) );
    }

    std::map<std::string, std::string, std::less<>> contentCache;
    std::map<std::string, c4::yml::Tree, std::less<>> yamlCache;
  };
}