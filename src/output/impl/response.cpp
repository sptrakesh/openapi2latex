//
// Created by Rakesh on 25/09/2025.
//

#include <boost/lexical_cast.hpp>


#include "output.hpp"

using std::operator ""sv;

namespace
{
  namespace presponse
  {
    void writeLink( std::string_view name, const spt::model::Link& link, std::ofstream& file )
    {
      auto line = R"(\subsection{)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      file.write( name.data(), static_cast<std::streamsize>( name.size() ) );
      file.write( "}\n", 2 );

      spt::output::impl::writeDescription( link, file );

      if ( !link.operationId.empty() )
      {
        const auto id = std::format( "operation::{}", link.operationId );
        line = R"(
\textbf{Operation} - see section \ref{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( id.data(), static_cast<std::streamsize>( id.size() ) );
        line = R"(} on page {\pageref{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( id.data(), static_cast<std::streamsize>( id.size() ) );
        file.write( "}.\n", 3 );
      }
      else if ( !link.operationRef.empty() )
      {
        const auto label = std::format( "operation::{}", link.operationRef );
        line = R"(
See chapter \ref{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        const auto cleaned = spt::output::impl::clean( link.operationRef );
        file.write( cleaned.data(), static_cast<std::streamsize>( cleaned.size() ) );
        file.write( "\n", 1 );
      }

      if ( link.requestBody.has_value() )
      {
        line = R"(\textbf{requestBody}
\begin{lstlisting}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        const auto str = std::any_cast<std::string>( link.requestBody );
        file.write( str.data(), static_cast<std::streamsize>( str.size() ) );
        line = R"(\end{lstlisting}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( !link.parameters.empty() )
      {
        line = R"(\begin{itemize}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

        for ( const auto& [pname, param] : link.parameters )
        {
          if ( !param.has_value() ) continue;
          line = R"(\item \textbf{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          const auto cleaned = spt::output::impl::clean( pname );
          file.write( cleaned.data(), static_cast<std::streamsize>( cleaned.size() ) );
          line = R"(} - \verb|)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          const auto str = std::any_cast<std::string>( param );
          file.write( str.data(), static_cast<std::streamsize>( str.size() ) );
          file.write( "|\n", 2 );
        }

        line = R"(\end{itemize}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( link.server )
      {
        line = R"(\textbf{server}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        spt::output::impl::writeDescription( *link.server, file );

        if ( !link.server->url.empty() )
        {
          line = R"(\href{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          auto str = boost::lexical_cast<std::string>( link.server->url );
          file.write( str.data(), static_cast<std::streamsize>( str.size() ) );
          line = R"(}{URL})"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        }
      }
    }

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
        for ( const auto& [cn, mt] : response.content ) spt::output::impl::writeMediaType( cn, mt, file );
      }

      if ( !response.links.empty() )
      {
        line = R"(\section{Links}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        for ( const auto& [ln, link] : response.links ) writeLink( ln, link, file );
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
