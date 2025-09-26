//
// Created by Rakesh on 25/09/2025.
//

#include "output.hpp"

using std::operator ""sv;

namespace
{
  namespace presponse
  {
    void writeResponse( std::string_view name, const spt::model::Response& response, std::ofstream& file )
    {
      const auto key = spt::output::impl::referenceKey( response, "response" );
      auto line = R"(\chapter{\label{)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      file.write( key.data(), static_cast<std::streamsize>( key.size() ) );
      file.write( "}", 1 );
      file.write( name.data(), static_cast<std::streamsize>( name.size() ) );
      file.write( "}\n", 2 );

      if ( !response.headers.empty() )
      {
        line = R"(\section{Headers}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

        for ( const auto& [hn, head] : response.headers ) spt::output::impl::writeHeader( hn, head, file );
      }

      if ( !response.content.empty() )
      {
        line = R"(\section{Content}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        for ( const auto& [hn, mt] : response.content ) spt::output::impl::writeMediaType( hn, mt, file );
      }
    }
  }
}

std::expected<std::filesystem::path, std::string> spt::output::impl::responses( const model::OpenAPI& openapi, std::filesystem::path path )
{
  using O = std::expected<std::filesystem::path, std::string>;
  if ( !openapi.components || openapi.components->responses.empty() ) return O{ std::unexpect, "No components/responses defined." };

  path.append( "responses.tex" );
  auto file = std::ofstream{ path };

  auto line = R"(\part{Responses}
)"sv;
  file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

  for ( const auto& [name, response] : openapi.components->responses ) presponse::writeResponse( name, response, file );

  return O{ std::in_place, path };
}
