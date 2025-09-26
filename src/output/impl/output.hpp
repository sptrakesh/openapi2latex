//
// Created by Rakesh on 25/09/2025.
//

#pragma once

#include "model/configuration.hpp"
#include "model/openapi.hpp"
#include "output/output.hpp"

#include <expected>
#include <filesystem>
#include <fstream>
#include <boost/algorithm/string/replace.hpp>

namespace spt::output::impl
{
  void writeInput( const std::filesystem::path& path, std::ofstream& file );
  std::string clean( std::string text );
  std::string schemaTitle( const model::Schema& schema );

  std::filesystem::path writeSchema( std::string_view key, const model::Schema& schema, std::filesystem::path path );
  std::expected<std::filesystem::path, std::string> responses( const model::OpenAPI& openapi, std::filesystem::path path );

  void tags( const std::filesystem::path& parent, const model::OpenAPI& openapi, std::ofstream& mainFile, const model::Configuration& conf );
  void writeHeader( std::string_view name, const model::Header& header, std::ofstream& file );
  void writeMediaType( std::string_view key, const model::MediaType& mt, std::ofstream& file );

  void schemaExamples( const model::Schema& schema, std::ofstream& file );
  void writeSchemaForAggregation( const model::Schema& schema, std::string_view title, std::ofstream& file );
  void writeSchemaProperties( const model::Schema& schema, std::ofstream& file );
  void writeSchemaAggregations( const model::Schema& schema, std::ofstream& file, bool eol = true );

  void writeExample( const model::Example& example, std::ofstream& file );
  void writeParameter( const model::Parameter& param, std::ofstream& file, bool initial = false );

  template <typename T>
  concept HasName = requires( T t )
  {
    std::is_same_v<decltype(t.name), std::string>;
  };

  template <typename T>
  concept HasTitle = requires( T t )
  {
    std::is_same_v<decltype(t.title), std::string>;
  };

  template <typename T>
  std::string referenceKey( const T& model, std::string_view prefix )
  {
    if constexpr ( HasName<T> )
    {
      if ( model._referenceURI.empty() ) return std::format( "{}::{}", prefix, model.name );
      return std::format( "{}::{}", prefix, std::hash<std::string>{}( model._referenceURI ) );
    }
    if constexpr ( HasTitle<T> )
    {
      if ( model._referenceURI.empty() ) return std::format( "{}::{}", prefix, model.title );
      return std::format( "{}::{}", prefix, std::hash<std::string>{}( model._referenceURI ) );
    }

    return std::format( "{}::{}", prefix, std::hash<std::string>{}( model._referenceURI ) );
  }

  template <typename T>
  void writeSummary( const T& model, std::ofstream& file )
  {
    using std::operator ""sv;

    if ( model.summary.empty() ) return;

    const auto sum = convert( model.summary );
    auto line = R"(\begin{quote})"sv;
    file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
    file.write( sum.data(), static_cast<std::streamsize>( sum.size() ) );
    line = R"(\end{quote}
)"sv;
    file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
  }

  template <typename T>
  void writeDescription( const T& model, std::ofstream& file )
  {
    using std::operator ""sv;

    if ( model.description.empty() ) return;

    const auto desc = convert( model.description );
    file.write( desc.data(), static_cast<std::streamsize>( desc.size() ) );
    file.write( "\n\n", 2 );
  }

  template <typename T>
  void writeSchema( const T& param, std::ofstream& file )
  {
    using std::operator ""sv;

    auto line = R"(\item \textit{schema})"sv;
    file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

    if ( !param.schema->description.empty() )
    {
      line = R"( - )"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      const auto desc = spt::output::convert( param.schema->description );
      file.write( desc.data(), static_cast<std::streamsize>( desc.size() ) );
    }
    file.write( "\n", 1 );

    line = R"(\begin{description}
)"sv;
    file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
    line = R"(\item \textit{type} - )"sv;
    file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
    file.write( param.schema->type.data(), static_cast<std::streamsize>( param.schema->type.size() ) );
    file.write( "\n", 1 );

    if ( !param.schema->enumeration.empty() )
    {
      auto names = std::format( "{:n}", param.schema->enumeration );
      boost::algorithm::replace_all( names, "\"", "" );
      line = R"(\item \textit{enum} - Allowed values )"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      file.write( names.data(), static_cast<std::streamsize>( names.size() ) );
      line = R"(}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
    }

    if ( param.schema->maximum.has_value() )
    {
      line = R"(\item \textit{maximum} - )"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      const auto v = std::format( "{}", *param.schema->maximum );
      file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
      file.write( "\n", 1 );
    }

    if ( param.schema->minimum.has_value() )
    {
      line = R"(\item \textit{minimum} - )"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      const auto v = std::format( "{}", *param.schema->minimum );
      file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
      file.write( "\n", 1 );
    }

    if ( !param.schema->pattern.empty() )
    {
      line = R"(\item \textit{pattern} - )"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      file.write( param.schema->pattern.data(), static_cast<std::streamsize>( param.schema->pattern.size() ) );
      file.write( "\n", 1 );
    }

    if ( !param.schema->format.empty() )
    {
      line = R"(\item \textit{format} - )"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      file.write( param.schema->format.data(), static_cast<std::streamsize>( param.schema->format.size() ) );
      file.write( "\n", 1 );
    }

    if ( param.schema->example.has_value() )
    {
      line = R"(\item \textit{example} - )"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      const auto v = std::any_cast<std::string>( param.schema->example );
      file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
      file.write( "\n", 1 );
    }

    for ( const auto& any : param.schema->examples )
    {
      line = R"(\item \textit{Examples} - )"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      const auto v = std::any_cast<std::string>( any );
      file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
      file.write( "\n", 1 );
    }

    if ( param.schema->_default.has_value() )
    {
      line = R"(\item \textit{default} - )"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      const auto v = std::any_cast<std::string>( param.schema->_default );
      file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
      file.write( "\n", 1 );
    }

    if ( !param.schema->sinceVersion.empty() )
    {
      line = R"(\item \textit{Since Version} - )"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      file.write( param.schema->sinceVersion.data(), static_cast<std::streamsize>( param.schema->sinceVersion.size() ) );
      file.write( "\n", 1 );
    }

    line = R"(\end{description}
)"sv;
    file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
  }
}