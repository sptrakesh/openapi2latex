//
// Created by Rakesh on 26/09/2025.
//

#include "collectors.hpp"
#include "output.hpp"
#include "log/NanoLog.hpp"
#include "util/split.hpp"

#include <set>
#include <boost/lexical_cast.hpp>

using std::operator ""s;
using std::operator ""sv;

namespace
{
  namespace ptag
  {
    std::string entityTitle( const spt::model::RequestBody& body )
    {
      auto title = body.ref.empty() ? "Request Body"s : body.ref;
      auto parts = spt::util::split( title, 4, "#/" );
      if ( !parts.empty() ) title = parts.back();
      return title;
    }

    void writeRequestBodyDetails( const spt::model::RequestBody& body, std::ofstream& file, std::string_view idPrefix, std::string_view operationId )
    {
      if ( !body.description.empty() )
      {
        const auto desc = spt::output::convert( body.description );
        file.write( desc.data(), static_cast<std::streamsize>( desc.size() ) );
        file.write( "\n", 1 );
      }

      const auto beginTable = [&file, idPrefix, operationId]
      {
        auto line = R"(\begin{center}
\tablefirsthead{%
\hline
\multicolumn{1}{|c}{\textbf{Property}} &
\multicolumn{1}{|c|}{\textbf{Value}} \\
\hline}
\tablehead{%
\hline
\multicolumn{2}{|c|}{continued from previous page}\\
\hline}
\tabletail{%
\hline
\multicolumn{2}{|c|}{continued on next page}\\
\hline
}
\tablelasttail{\hline}
\tablecaption{\label{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

        const auto key = std::format( "{}::request::table", idPrefix );
        file.write( key.data(), static_cast<std::streamsize>( key.size() ) );
        line = R"(}Request body for )"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( operationId.data(), static_cast<std::streamsize>( operationId.size() ) );
        line = R"(}
  \begin{supertabular}{|l|p{100mm}|}
  )"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      };

      const auto endTable = [&file]
      {
        auto line = R"(
\end{supertabular}
\end{center}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      };

      beginTable();

      auto line = R"(Required & )"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      const auto required = std::format( "{}", body.required );
      file.write( required.data(), static_cast<std::streamsize>( required.size() ) );
      line = R"(\\
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

      if ( !body.content.empty() )
      {
        line = R"(\hline Content &)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

        for ( const auto& [mkey, mt] : body.content ) spt::output::impl::writeMediaType( mkey, mt, file );

        line = R"(\\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      endTable();
    }
    void writeRequestBody( const spt::model::RequestBody& body, std::ofstream& file, std::string_view idPrefix, std::string_view operationId )
    {
      const auto title = entityTitle( body );

      if ( !body._referenceURI.empty() )
      {
        const auto key = std::format( "requestBody::{}", std::hash<std::string>{}( body._referenceURI ) );
        if ( spt::output::impl::Collectors::instance().requestBodyMap.contains( key ) )
        {
          auto line = R"(\subsection*{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( title.data(), static_cast<std::streamsize>( title.size() ) );
          line = R"(}
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

          if ( !body.description.empty() )
          {
            const auto desc = spt::output::convert( body.description );
            file.write( desc.data(), static_cast<std::streamsize>( desc.size() ) );
            file.write( "\n", 1 );
          }

          line = R"(See section \ref{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( key.data(), static_cast<std::streamsize>( key.size() ) );
          line = R"(} on page \pageref{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( key.data(), static_cast<std::streamsize>( key.size() ) );
          line = R"(}.
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          return;
        }

        spt::output::impl::Collectors::instance().requestBodyMap.try_emplace( key, std::cref( body ) );
        auto line = R"(\subsection{\label{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( key.data(), static_cast<std::streamsize>( key.size() ) );
        file.write( "}", 1 );
        file.write( title.data(), static_cast<std::streamsize>( title.size() ) );
        line = R"(}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }
      else
      {
        const auto key = std::format( "{}:requestBody", idPrefix );
        auto line = R"(\subsection{\label{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( key.data(), static_cast<std::streamsize>( key.size() ) );
        file.write( "}", 1 );
        file.write( title.data(), static_cast<std::streamsize>( title.size() ) );
        line = R"(}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      writeRequestBodyDetails( body, file, idPrefix, operationId );
    }

    void writeOperationSummary( const spt::model::Operation& operation, std::string_view id, std::ofstream& file, const spt::model::Configuration& conf )
    {
      if ( operation.summary.empty() )
      {
        auto line = R"(
\section{\label{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( id.data(), static_cast<std::streamsize>( id.size() ) );
        file.write( "}", 1 );
        file.write( operation.operationId.data(), static_cast<std::streamsize>( operation.operationId.size() ) );
        line = R"(}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }
      else if ( conf.operationSummary )
      {
        auto cs = spt::output::convert( operation.summary );
        if ( cs.size() > 80 )
        {
          auto line = R"(\section[)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( id.data(), static_cast<std::streamsize>( id.size() ) );
          line = R"(]{\label{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( id.data(), static_cast<std::streamsize>( id.size() ) );
          file.write( "}", 1 );
          file.write( cs.data(), static_cast<std::streamsize>( cs.size() ) );
          line = R"(}
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        }
        else
        {
          auto line = R"(\section{\label{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( id.data(), static_cast<std::streamsize>( id.size() ) );
          file.write( "}", 1 );
          file.write( cs.data(), static_cast<std::streamsize>( cs.size() ) );
          line = R"(}
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        }
      }
      else
      {
        auto line = R"(\section{\label{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( id.data(), static_cast<std::streamsize>( id.size() ) );
        file.write( "}", 1 );
        file.write( operation.operationId.data(), static_cast<std::streamsize>( operation.operationId.size() ) );
        line = R"(}
\index{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( operation.operationId.data(), static_cast<std::streamsize>( operation.operationId.size() ) );
        file.write( "}\n", 2 );

        if ( !operation.summary.empty() )
        {
          line = R"(\begin{quote})"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          auto cs = spt::output::convert( operation.summary );
          file.write( cs.data(), static_cast<std::streamsize>( cs.size() ) );
          line = R"(\end{quote}
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        }
      }
    }

    void writeOperationTable( const spt::model::Operation& operation, std::string_view path, std::string_view method, std::ofstream& file )
    {
      auto line = R"(\begin{minipage}{\textwidth}
\tablefirsthead{}
\tablehead{}
\tabletail{}
\begin{supertabular}{l|l}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      line = R"(Resource Path & \seqsplit{)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      file.write( path.data(), static_cast<std::streamsize>( path.size() ) );
      line = R"(} \\
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

      line = R"(HTTP Method & )"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      file.write( method.data(), static_cast<std::streamsize>( method.size() ) );
      line = R"( \\
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

      for ( const auto& srv : operation.servers )
      {
        auto url = boost::lexical_cast<std::string>( srv.url );
        file.write( srv.description.data(), static_cast<std::streamsize>( srv.description.size() ) );
        line = R"( & \href{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( url.data(), static_cast<std::streamsize>( url.size() ) );
        file.write( path.data(), static_cast<std::streamsize>( path.size() ) );
        file.write( "}{", 2 );
        file.write( url.data(), static_cast<std::streamsize>( url.size() ) );
        line = R"(} \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( operation.security.empty() )
      {
        line = R"(Security & None\\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }
      else
      {
        auto names = std::vector<std::string>{};
        names.reserve( operation.security.size() );
        for ( const auto& sr : operation.security )
        {
          for ( const auto& [key, _] : sr.values ) names.push_back( key );
        }

        if ( !names.empty() )
        {
          line = R"(Security & )"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          auto value = std::format( "{:n}", names );
          boost::replace_all( value, "\"", "" );
          value = spt::output::impl::clean( value );
          file.write( value.data(), static_cast<std::streamsize>( value.size() ) );
          line = R"(\footnote{See table \ref{table::security::schemes} on page \pageref{table::security::schemes}}\\
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        }
      }

      if ( !operation.sinceVersion.empty() )
      {
        line = R"(Since Version & )"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( operation.sinceVersion.data(), static_cast<std::streamsize>( operation.sinceVersion.size() ) );
        line = R"( \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( operation.deprecated )
      {
        line = R"(\textit{Deprecated} & true\\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      line = R"(\end{supertabular}
\end{minipage}

)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
    }

    void writeOperationDocs( const spt::model::Operation& operation, std::ofstream& file )
    {
      if ( operation.externalDocs )
      {
        if ( !operation.externalDocs->description.empty() )
        {
          auto desc = spt::output::convert( operation.externalDocs->description );
          file.write( desc.data(), static_cast<std::streamsize>( desc.size() ) );
          file.write( "\n", 1 );
        }
        if ( !operation.externalDocs->url.empty() )
        {
          auto url = boost::lexical_cast<std::string>( operation.externalDocs->url );
          auto line = R"(\url{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          line = R"(}
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( url.data(), static_cast<std::streamsize>( url.size() ) );
          file.write( "\n", 1 );
        }
      }
    }

    void writeOperationParameters( const spt::model::Operation& operation, std::string_view id, std::ofstream& file )
    {
      if ( !operation.parameters.empty() )
      {
        auto line = R"(\subsection{\label{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( id.data(), static_cast<std::streamsize>( id.size() ) );
        line = R"(:parameters}Parameters}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

        line = R"(\begin{itemize}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

        for ( const auto& param : operation.parameters ) spt::output::impl::writeParameter( param, file );

        line = R"(\end{itemize}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }
    }

    void writeOperationResponse( std::string_view code, const spt::model::Response& response, std::ofstream& file )
    {
      for ( const auto& [ct, sc] : response.content )
      {
        if ( !sc.schema ) continue;

        auto desc = spt::output::convert( sc.schema->description );
        if ( !desc.empty() ) desc.append( "\n\n" );
        const auto title = spt::output::impl::schemaTitle( *sc.schema );

        auto line = R"(\hline )"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( code.data(), static_cast<std::streamsize>( code.size() ) );
        line = R"( & )"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( ct.data(), static_cast<std::streamsize>( ct.size() ) );
        line = R"( & )"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

        if ( !sc.schema->_referenceURI.empty() )
        {
          const auto key = spt::output::impl::referenceKey( *sc.schema, "schema"sv );
          file.write( desc.data(), static_cast<std::streamsize>( desc.size() ) );
          line = R"(\textbf{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( title.data(), static_cast<std::streamsize>( title.size() ) );
          line = R"(}. See chapter \ref{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( key.data(), static_cast<std::streamsize>( key.size() ) );
          line = R"(} on page \pageref{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( key.data(), static_cast<std::streamsize>( key.size() ) );
          line = R"(} for schema. \\
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          continue;
        }

        if ( !response.description.empty() )
        {
          const auto sum = spt::output::convert( response.description );
          file.write( sum.data(), static_cast<std::streamsize>( sum.size() ) );
          file.write( "\n\n", 2 );
        }

        if ( !sc.schema->summary.empty() )
        {
          const auto sum = spt::output::convert( sc.schema->summary );
          file.write( sum.data(), static_cast<std::streamsize>( sum.size() ) );
          file.write( "\n\n", 2 );
        }

        if ( !desc.empty() )
        {
          file.write( desc.data(), static_cast<std::streamsize>( desc.size() ) );
          file.write( "\n\n", 2 );
        }

        spt::output::impl::writeSchemaProperties( *sc.schema, file );
        spt::output::impl::writeSchemaAggregations( *sc.schema, file );
      }
    }

    void writeOperationExamples( const spt::model::Operation& operation, std::ofstream& file )
    {
      auto started = false;

      const auto start = [&started, &file]
      {
        if ( !started )
        {
          auto line = R"(\subsubsection{Examples}
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          started = true;
        }
      };

      for ( const auto& [code, resp] : operation.responses )
      {
        for ( const auto& [ct, schema] : resp.content )
        {
          if ( schema.example.has_value() )
          {
            start();
            const auto ex = std::any_cast<std::string>( schema.example );
            auto line = R"(\begin{lstlisting}
)"sv;
            file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
            file.write( ex.data(), static_cast<std::streamsize>( ex.size() ) );
            line = R"(\end{lstlisting}

)";
            file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          }
          else
          {
            for ( const auto& [et, ex] : schema.examples )
            {
              if ( !ex.value.has_value() ) continue;

              start();
              auto line = R"(\textbf{\large )"sv;
              file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
              file.write( et.data(), static_cast<std::streamsize>( et.size() ) );
              file.write( "}\n", 2 );

              if ( !ex.summary.empty() )
              {
                const auto sum = spt::output::convert( ex.summary );
                file.write( sum.data(), static_cast<std::streamsize>( sum.size() ) );
                file.write( "\n\n", 2 );
              }

              if ( !ex.description.empty() )
              {
                const auto sum = spt::output::convert( ex.description );
                file.write( sum.data(), static_cast<std::streamsize>( sum.size() ) );
                file.write( "\n\n", 2 );
              }

              const auto value = std::any_cast<std::string>( ex.value );
              file.write( value.data(), static_cast<std::streamsize>( value.size() ) );
              file.write( "\n\n", 2 );
            }
          }
        }
      }

      if ( !operation.codeSamples.empty() )
      {
        auto line = R"(\subsubsection{Code Samples}
See section \ref{codesamples:)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( operation.operationId.data(), static_cast<std::streamsize>( operation.operationId.size() ) );
        line = R"(} on page \pageref{codesamples:)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( operation.operationId.data(), static_cast<std::streamsize>( operation.operationId.size() ) );
        file.write( "}\n", 2 );
      }
    }

    void writeOperation( const spt::model::Operation& operation, const spt::model::Tag& tag, std::string_view apiPath,
      std::ofstream& file, const spt::model::Configuration& conf, std::string_view method )
    {
      static auto oplabels = std::set<std::string, std::less<>>{};

      if ( operation.operationId.empty() ) return;
      if ( const auto iter = std::ranges::find( operation.tags, tag.name ); iter == std::ranges::end( operation.tags ) ) return;

      auto path = std::string{ apiPath };
      boost::algorithm::replace_all( path, "{", "\\{" );
      boost::algorithm::replace_all( path, "}", "\\}" );

      const auto id = std::format( "operation::{}", operation.operationId );

      // Only show reference for operations that have been added with another tag
      if ( oplabels.contains( id ) )
      {
        auto line = R"(\section*{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( operation.operationId.data(), static_cast<std::streamsize>( operation.operationId.size() ) );
        line = R"(}
\index{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( operation.operationId.data(), static_cast<std::streamsize>( operation.operationId.size() ) );
        file.write( "}\n", 2 );

        line = R"(\seqsplit{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( path.data(), static_cast<std::streamsize>( path.size() ) );
        line = R"(}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

        line = R"(See section \ref{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( id.data(), static_cast<std::streamsize>( id.size() ) );
        line = R"(} on page \pageref{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( id.data(), static_cast<std::streamsize>( id.size() ) );
        line = R"(}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        return;
      }

      oplabels.insert( id );
      writeOperationSummary( operation, id, file, conf );
      writeOperationTable( operation, path, method, file );

      if ( !operation.description.empty() )
      {
        auto desc = spt::output::convert( operation.description );
        file.write( desc.data(), static_cast<std::streamsize>( desc.size() ) );
      }

      writeOperationDocs( operation, file );
      writeOperationParameters( operation, id, file );

      if ( operation.requestBody ) writeRequestBody( *operation.requestBody, file, id, operation.operationId );
      if ( !operation.responses.empty() )
      {
        auto line = R"(\subsection{\label{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( id.data(), static_cast<std::streamsize>( id.size() ) );
        line = R"(:responses}Responses}
See table \ref{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( id.data(), static_cast<std::streamsize>( id.size() ) );
        line = R"(:responses:table} for response codes and data.
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

        line = R"(\begin{center}
\tablefirsthead{%
\hline
\multicolumn{1}{|c}{\textbf{Code}} &
\multicolumn{1}{|c}{\textbf{Content Type}} &
\multicolumn{1}{|c|}{\textbf{Notes}} \\}
\tablehead{%
\hline
\multicolumn{3}{|c|}{continued from previous page}\\}
\tabletail{%
\hline
\multicolumn{3}{|c|}{continued on next page}\\
\hline
}
\tablelasttail{\hline})"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        line = R"(\tablecaption{\label{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( id.data(), static_cast<std::streamsize>( id.size() ) );
        line = R"(:responses:table}Responses for )"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( operation.operationId.data(), static_cast<std::streamsize>( operation.operationId.size() ) );
        line = R"(}
\begin{supertabular}{|l|l|p{80mm}|}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

        for ( const auto& [code, resp] : operation.responses ) writeOperationResponse( code, resp, file );

        line = R"(
\end{supertabular}
\end{center}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      writeOperationExamples( operation, file );
    }
  }
}

void spt::output::impl::tags( const std::filesystem::path& parent, const spt::model::OpenAPI& openapi, std::ofstream& mainFile, const spt::model::Configuration& conf )
{
  if ( openapi.tags.empty() ) return;

  for ( const auto& tag : openapi.tags )
  {
    LOG_DEBUG << "Adding operations for tag " << tag.name;
    auto path = parent;
    path.append( std::format( "tag-{}.tex", tag.name ) );
    auto file = std::ofstream{ path };

    auto line = R"(\chapter{\label{tag::)"sv;
    file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
    file.write( tag.name.data(), static_cast<std::streamsize>( tag.name.size() ) );
    file.write( "}", 1 );
    file.write( tag.name.data(), static_cast<std::streamsize>( tag.name.size() ) );
    line = R"(}
\index{)"sv;
    file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
    file.write( tag.name.data(), static_cast<std::streamsize>( tag.name.size() ) );
    file.write( "}\n", 2 );

    if ( !tag.description.empty() )
    {
      line = R"(\begin{quote}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      auto data = spt::output::convert( tag.description );
      file.write( data.data(), static_cast<std::streamsize>( data.size() ) );
      line = R"(\end{quote}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
    }

    for ( const auto& [key, pi] : openapi.paths )
    {
      if ( pi.post ) ptag::writeOperation( *pi.post, tag, key, file, conf, "POST"sv );
      if ( pi.put ) ptag::writeOperation( *pi.put, tag, key, file, conf, "PUT"sv );
      if ( pi.get ) ptag::writeOperation( *pi.get, tag, key, file, conf, "GET"sv );
      if ( pi.patch ) ptag::writeOperation( *pi.patch, tag, key, file, conf, "PATCH"sv );
      if ( pi._delete ) ptag::writeOperation( *pi._delete, tag, key, file, conf, "DELETE"sv );
      if ( pi.head ) ptag::writeOperation( *pi.head, tag, key, file, conf, "HEAD"sv );
      if ( pi.options ) ptag::writeOperation( *pi.options, tag, key, file, conf, "OPTIONS"sv );
      if ( pi.trace ) ptag::writeOperation( *pi.trace, tag, key, file, conf, "TRACE"sv );
    }

    writeInput( path, mainFile );
  }
}
