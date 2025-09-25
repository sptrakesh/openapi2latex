//
// Created by Rakesh on 22/09/2025.
//

#include "output.hpp"
#include "log/NanoLog.hpp"
#include "util/split.hpp"

#include <cstdlib>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <ranges>
#include <set>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>

using std::operator ""s;
using std::operator ""sv;
constexpr std::string_view preambleContents =
#include "../../preamble.tex"
;

namespace
{
  namespace poutput
  {
    std::map<std::string, std::reference_wrapper<const spt::model::Parameter>> parameterMap{};
    std::map<std::string, std::reference_wrapper<const spt::model::RequestBody>> requestBodyMap{};

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

      return {};
    }

    std::string schemaTitle( const spt::model::Schema& schema )
    {
      if ( !schema.title.empty() ) return schema.title;
      if ( schema.ref.empty() ) return "Schema"s;
      auto parts = spt::util::split( schema.ref, 4, "#/" );
      return parts.empty() ? "Schema"s : std::string{ parts.back() };
    }

    std::string clean( std::string text )
    {
      boost::replace_all( text, "&", R"(\&)" );
      boost::replace_all( text, R"($)", R"(\$)" );
      boost::replace_all( text, R"([)", R"(\[)" );
      boost::replace_all( text, R"(])", R"(\])" );
      boost::replace_all( text, R"({)", R"(\})" );
      boost::replace_all( text, R"(_)", R"(\textunderscore )" );
      boost::replace_all( text, R"(#)", R"(\#)" );
      boost::replace_all( text, R"(%)", R"(\%)" );
      return text;
    }

    void writeInput( const std::filesystem::path& path, std::ofstream& file )
    {
      auto line = R"(\input{)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      auto input = path.string();
      file.write( input.data(), static_cast<std::streamsize>( input.size() ) );
      line = R"(}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
    }

    std::filesystem::path preamble( std::filesystem::path path )
    {
      path.append( "preamble.tex" );
      auto file = std::ofstream{ path };
      file.write( preambleContents.data(), preambleContents.size() );
      file.close();
      return path;
    }

    std::filesystem::path frontmatter( std::filesystem::path path, spt::model::OpenAPI& openapi, const spt::model::Configuration& conf )
    {
      path.append( "frontmatter.tex" );
      auto file = std::ofstream{ path };

      auto lines = R"(
\lhead{\textsf{\textbf{OpenAPI2\LaTeX}}}
\lfoot{\textsf{\textbf{Version #VERSION#}}}
\rfoot{\textsf{\textbf{#FOOTER#}}}

\begin{document}

\title{#TITLE#\\
Version: #VERSION#}
\author{#AUTHOR#}
\date{\today}
\maketitle
% Title Page

\frontmatter
%\thispagestyle{empty}
%\newpage
\tableofcontents

\clearpage
\mainmatter)"s;
      boost::algorithm::replace_all( lines, "#VERSION#", openapi.info.version.empty() ? openapi.openapi : openapi.info.version );
      boost::algorithm::replace_all( lines, "#FOOTER#", conf.footer );
      boost::algorithm::replace_all( lines, "#AUTHOR#", conf.author );
      boost::algorithm::replace_all( lines, "#TITLE#", openapi.info.title );
      file.write( lines.data(), static_cast<std::streamsize>( lines.size() ) );
      file.close();
      return path;
    }

    void table( const spt::model::Info& info, std::ofstream& file )
    {
      if ( info.termsOfService.empty() && info.version.empty() && ( !info.license || info.license->name.empty() ) ) return;

      auto line = R"(\section{Other Information}
\begin{center}
\tablefirsthead{}
\tablehead{}
\tabletail{}
\tablecaption{API Information}
\begin{supertabular}{|l|l|}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

      if ( !info.termsOfService.scheme().empty() )
      {
        line = R"(\hline Terms Of Service & \url{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        auto link = boost::lexical_cast<std::string>( info.termsOfService );
        file.write( link.data(), static_cast<std::streamsize>( link.size() ) );
        line = R"(} \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( !info.version.empty() )
      {
        line = R"(\hline Version & )"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( info.version.data(), static_cast<std::streamsize>( info.version.size() ) );
        line = R"( \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( info.license )
      {
        if ( !info.license->identifier.empty() )
        {
          line = R"(\hline License & )"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( info.license->identifier.data(), static_cast<std::streamsize>( info.license->identifier.size() ) );
          line = R"(} \\
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        }
        else if ( !info.license->name.empty() && !info.license->url.scheme().empty() )
        {
          line = R"(\hline License & \href{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          auto link = boost::lexical_cast<std::string>( info.license->url );
          file.write( link.data(), static_cast<std::streamsize>( link.size() ) );
          line = "}{"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( info.license->name.data(), static_cast<std::streamsize>( info.license->name.size() ) );

          line = R"(} \\
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        }
      }

      line = R"(\hline
\end{supertabular}
\end{center})"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
    }

    void servers( const spt::model::OpenAPI& openapi, std::ofstream& file )
    {
      if ( openapi.servers.empty() ) return;

      auto line = R"(\begin{center}
\tablefirsthead{}
\tablehead{}
\tabletail{}
\tablecaption{Server Information}
\begin{supertabular}{|l|l|}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

      for ( const auto& server : openapi.servers )
      {
        line = R"(\hline )"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( server.description.data(), static_cast<std::streamsize>( server.description.size() ) );
        line = R"(& \url{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        auto url = boost::lexical_cast<std::string>( server.url );
        file.write( url.data(), static_cast<std::streamsize>( url.size() ) );
        line = R"(} \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      line = R"(\hline
\end{supertabular}
\end{center}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
    }

    void securitySchemes( const spt::model::Components& components, std::ofstream& file )
    {
      if ( components.securitySchemes.empty() ) return;

      auto line = R"(
\begin{center}
\tablefirsthead{}
\tablehead{}
\tabletail{}
\tablecaption{\label{table::security::schemes}Security Schemes}
\begin{supertabular}{|l|l|l|}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

      for ( const auto& [key, scheme] : components.securitySchemes )
      {
        auto count = 0;
        if ( scheme.type.empty() ) ++count;
        if ( scheme.description.empty() ) ++count;
        if ( scheme.name.empty() ) ++count;
        if ( scheme.in.empty() ) ++count;
        if ( scheme.scheme.empty() ) ++count;
        if ( scheme.bearerFormat.empty() ) ++count;
        if ( scheme.openIdConnectUrl.empty() ) ++count;

        line = R"(\hline \multirow{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        auto cs = std::to_string( count );
        file.write( cs.data(), static_cast<std::streamsize>( cs.size() ) );

        line = R"(}{*}{\textbf{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( key.data(), static_cast<std::streamsize>( key.size() ) );

        line = R"(}} & Type & )"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( scheme.type.data(), static_cast<std::streamsize>( scheme.type.size() ) );
        line = R"( \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

        if ( !scheme.description.empty() )
        {
          line = R"(\cline{2-3} & Description & )"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          auto desc = spt::output::convert( scheme.description );
          file.write( desc.data(), static_cast<std::streamsize>( desc.size() ) );
          line = R"( \\
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        }

        if ( !scheme.name.empty() )
        {
          line = R"(\cline{2-3} & Name & )"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          auto desc = spt::output::convert( scheme.name );
          file.write( desc.data(), static_cast<std::streamsize>( desc.size() ) );
          line = R"( \\
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        }

        if ( !scheme.in.empty() )
        {
          line = R"(\cline{2-3} & In & )"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          auto desc = spt::output::convert( scheme.in );
          file.write( desc.data(), static_cast<std::streamsize>( desc.size() ) );
          line = R"( \\
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        }

        if ( !scheme.scheme.empty() )
        {
          line = R"(\cline{2-3} & Scheme & )"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          auto desc = spt::output::convert( scheme.scheme );
          file.write( desc.data(), static_cast<std::streamsize>( desc.size() ) );
          line = R"( \\
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        }

        if ( !scheme.bearerFormat.empty() )
        {
          line = R"(\cline{2-3} & Bearer Format & )"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          auto desc = spt::output::convert( scheme.bearerFormat );
          file.write( desc.data(), static_cast<std::streamsize>( desc.size() ) );
          line = R"( \\
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        }

        if ( !scheme.openIdConnectUrl.empty() )
        {
          line = R"(\cline{2-3} & OpenId Connect & \url{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          auto desc = boost::lexical_cast<std::string>( scheme.openIdConnectUrl );
          file.write( desc.data(), static_cast<std::streamsize>( desc.size() ) );
          line = R"(} \\
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        }
      }

      line = R"(\hline
\end{supertabular}
\end{center}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
    }

    void examples( const spt::model::Components& components, std::ofstream& file )
    {
      if ( components.examples.empty() ) return;

      auto line = R"(\chapter{Examples}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

      for ( const auto& [tag, example] : components.examples )
      {
        if ( !example.value.has_value() ) continue;
        line = R"(\section{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( tag.data(), static_cast<std::streamsize>( tag.size() ) );

        line = R"(}
\begin{quote}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        auto cleaned = spt::output::convert( example.summary );
        file.write( cleaned.data(), static_cast<std::streamsize>( cleaned.size() ) );

        line = R"(\end{quote}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

        cleaned = spt::output::convert( example.description );
        file.write( cleaned.data(), static_cast<std::streamsize>( cleaned.size() ) );

        line = R"(
\begin{lstlisting}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        auto code = std::any_cast<std::string>( example.value );
        file.write( code.data(), static_cast<std::streamsize>( code.size() ) );
        line = R"(
\end{lstlisting}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }
    }

    std::filesystem::path infoDescription( const std::filesystem::path& path, const spt::model::Info& info )
    {
      auto pmd = path.parent_path();
      pmd.append( "infodesc.md" );
      LOG_INFO << "Generating latex for description using cmark";

      auto mdfile = std::ofstream{ pmd };
      mdfile.write( info.description.data(), static_cast<std::streamsize>( info.description.size() ) );
      mdfile.close();

      auto tpath = path.parent_path();
      tpath.append( "infodesc.tex" );

      const auto cmd = std::format("cmark {} -t latex > {}", pmd.string(), tpath.string() );
      std::system( cmd.c_str() );
      return tpath;
    }

    std::filesystem::path info( std::filesystem::path path, const spt::model::OpenAPI& openapi, bool cmark )
    {
      path.append( "info.tex" );
      auto file = std::ofstream{ path };

      auto line = R"(\chapter{Information}
\begin{quote}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      auto summary = spt::output::convert( openapi.info.summary );
      file.write( summary.data(), static_cast<std::streamsize>( summary.size() ) );
      line = R"(\end{quote}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      file.write( "\n", 1 );

      if ( cmark )
      {
        auto dp = infoDescription( path, openapi.info );
        line = R"(\input{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        auto dstr = dp.string();
        file.write( dstr.data(), static_cast<std::streamsize>( dstr.size() ) );
        line = R"(}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }
      else
      {
        auto desc = spt::output::convert( openapi.info.description );
        file.write( desc.data(), static_cast<std::streamsize>( desc.size() ) );
        file.write( "\n", 1 );
      }

      table( openapi.info, file );

      file.close();
      return path;
    }

    std::expected<std::filesystem::path, std::string> tagGroups( std::filesystem::path path, const spt::model::OpenAPI& openapi )
    {
      using O = std::expected<std::filesystem::path, std::string>;

      if ( openapi.tagGroups.empty() ) return O{ std::unexpect, "No tag groups" };
      path.append( "taggroups.tex" );
      auto file = std::ofstream{ path };

      auto line = R"(\chapter{Tag Groups}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

      for ( const auto& tag : openapi.tagGroups )
      {
        line = R"(\section{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( tag.name.data(), static_cast<std::streamsize>( tag.name.size() ) );

        line = R"(}
\begin{itemize})"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

        for ( const auto& t : tag.tags )
        {
          line = R"(\item \hyperref[tag::)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( t.data(), static_cast<std::streamsize>( t.size() ) );
          line = R"(]{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( t.data(), static_cast<std::streamsize>( t.size() ) );
          line = R"(} on page \pageref{tag::)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( t.data(), static_cast<std::streamsize>( t.size() ) );
          line = R"(}
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        }

        line = R"(\end{itemize}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      file.close();
      return O{ std::in_place, path };
    }

    void schemaExamples( const spt::model::Schema& schema, std::ofstream& file )
    {
      if ( schema.type == "object" || schema.type == "array" ) return;

      if ( schema.example.has_value() )
      {
        auto line = R"(\item \textbf{Example} \verb|)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        const auto ex = std::any_cast<std::string>( schema.example );
        file.write( ex.data(), static_cast<std::streamsize>( ex.size() ) );
        file.write( "|\n", 2 );
        return;
      }

      for ( const auto& example : schema.examples )
      {
        if ( !example.has_value() ) continue;
        auto line = R"(\item \textbf{Example} \verb|)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        const auto ex = std::any_cast<std::string>( example );
        file.write( ex.data(), static_cast<std::streamsize>( ex.size() ) );
        file.write( "|\n", 2 );
      }
    }

    void writeSchemaProperties( const spt::model::Schema& schema, std::ofstream& file )
    {
      if ( schema.properties.empty() ) return;
      auto line = R"(\begin{itemize}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

      for ( const auto& [name, prop] : schema.properties )
      {
        const auto title = schemaTitle( prop );

        line = R"(\item \textbf{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        const auto cleaned = clean( name );
        file.write( cleaned.data(), static_cast<std::streamsize>( cleaned.size() ) );
        line = R"(}
\index{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( cleaned.data(), static_cast<std::streamsize>( cleaned.size() ) );
        file.write( "}\n", 2 );

        if ( !prop._referenceURI.empty() )
        {
          line = R"(\textbf{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( title.data(), static_cast<std::streamsize>( title.size() ) );

          const auto key = referenceKey( prop, "schema" );
          line = R"(}. See chapter \ref{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( key.data(), static_cast<std::streamsize>( key.size() ) );
          line = R"(} on page \pageref{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( key.data(), static_cast<std::streamsize>( key.size() ) );
          line = R"(} for schema.
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

          continue;
        }

        if ( !prop.description.empty() )
        {
          const auto desc = spt::output::convert( schema.description );
          file.write( desc.data(), static_cast<std::streamsize>( desc.size() ) );
          file.write( "\n", 1 );
        }

        line = R"(\begin{itemize}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

        line = R"(\item \textbf{Type} )"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( prop.type.data(), static_cast<std::streamsize>( prop.type.size() ) );
        file.write( "\n", 1 );

        if ( !prop.format.empty() )
        {
          line = R"(\item \textbf{Format} )"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( prop.format.data(), static_cast<std::streamsize>( prop.format.size() ) );
          file.write( "\n", 1 );
        }

        schemaExamples( prop, file );

        line = R"(\end{itemize}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      line = R"(\end{itemize}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
    }

    void writeSchemaForAggregation( const spt::model::Schema& schema, std::string_view title, std::ofstream& file )
    {
      auto line = R"(\item )"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      file.write( title.data(), static_cast<std::streamsize>( title.size() ) );
      line = R"( of type )"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      file.write( schema.type.data(), static_cast<std::streamsize>( schema.type.size() ) );
      file.write( "\n", 1 );

      const auto summary = [&file]( const spt::model::Schema& sc )
      {
        if ( !sc.summary.empty() )
        {
          const auto sum = spt::output::convert( sc.summary );
          auto line = R"(\begin{quote})"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( sum.data(), static_cast<std::streamsize>( sum.size() ) );
          line = R"(\end{quote}
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        }
      };

      const auto description = [&file]( const spt::model::Schema& sc )
      {
        if ( !sc.description.empty() )
        {
          const auto desc = spt::output::convert( sc.description );
          file.write( desc.data(), static_cast<std::streamsize>( desc.size() ) );
          file.write( "\n", 1 );
        }
      };

      const auto cleanedProperty = [&file]( std::string_view title, const std::string& value )
      {
        if ( value.empty() ) return;
        const auto cleaned = clean( value );
        auto line = R"(\item \textbf{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( title.data(), static_cast<std::streamsize>( title.size() ) );
        line = R"(} - )"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( cleaned.data(), static_cast<std::streamsize>( cleaned.size() ) );
        line = R"(
\index{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( title.data(), static_cast<std::streamsize>( title.size() ) );
        file.write( "}\n", 2 );
      };

      const auto boolean = [&file]( std::string_view title, bool value )
      {
        if ( !value ) return;
        auto line = R"(\item \textbf{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( title.data(), static_cast<std::streamsize>( title.size() ) );
        line = R"(} - \texttt{true}
\index{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( title.data(), static_cast<std::streamsize>( title.size() ) );
        file.write( "}\n", 2 );
      };

      const auto optdouble = [&file]( std::string_view title, std::optional<double> value )
      {
        if ( !value ) return;
        const auto v = std::format( "{}", *value );
        auto line = R"(\item \textbf{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( title.data(), static_cast<std::streamsize>( title.size() ) );
        line = R"(} - \texttt{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
        line = R"(}
\index{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( title.data(), static_cast<std::streamsize>( title.size() ) );
        file.write( "}\n", 2 );
      };

      summary( schema );
      description( schema );

      const std::function<void( const spt::model::Schema& prop, const std::string& name )> property = [&file, &summary, &description, &cleanedProperty, &boolean, &optdouble, &property]( const spt::model::Schema& prop, const std::string& name )
      {
        auto line = R"(\item \textbf{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( name.data(), static_cast<std::streamsize>( name.size() ) );
        line = R"(}
\index{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( name.data(), static_cast<std::streamsize>( name.size() ) );
        file.write( "}\n", 2 );

        summary( prop );

        if ( !prop._referenceURI.empty() )
        {
          const auto key = referenceKey( prop, "schema"sv );
          line = R"(See chapter \ref{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( key.data(), static_cast<std::streamsize>( key.size() ) );
          line = R"(} on page \pageref{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( key.data(), static_cast<std::streamsize>( key.size() ) );
          file.write( "}.\n", 3 );
          return;
        }

        description( prop );

        line = R"(
\begin{itemize}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

        line = R"(\item \textbf{type} - )"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( prop.type.data(), static_cast<std::streamsize>( prop.type.size() ) );

        cleanedProperty( "title"sv, prop.title );
        cleanedProperty( "sinceVersion"sv, prop.sinceVersion );
        cleanedProperty( "format"sv, prop.format );
        cleanedProperty( "dialect"sv, prop.dialect );

        if ( !prop.pattern.empty() )
        {
          line = R"(\item \textbf{pattern} - \verb|)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( prop.pattern.data(), static_cast<std::streamsize>( prop.pattern.size() ) );
          file.write( "|\n", 2 );
        }

        if ( const auto iter = std::ranges::find( prop.required, name ); iter != std::ranges::end( prop.required ) )
        {
          boolean( "required", true );
        }

        boolean( "nullable", prop.nullable );
        boolean( "readOnly", prop.readOnly );
        boolean( "writeOnly", prop.writeOnly );
        boolean( "deprecated", prop.deprecated );

        optdouble( "maximum", prop.maximum );
        optdouble( "exclusiveMaximum", prop.exclusiveMaximum );
        optdouble( "minimum", prop.minimum );
        optdouble( "exclusiveMinimum", prop.exclusiveMinimum );
        optdouble( "maxItems", prop.maxItems );
        optdouble( "minItems", prop.minItems );
        optdouble( "maxLength", prop.maxLength );
        optdouble( "minLength", prop.minLength );

        if ( !prop.enumeration.empty() )
        {
          line = R"(\item \textbf{enum} -
\being{description})"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

          for ( const auto& e : prop.enumeration )
          {
            line = R"(\item \texttt{")"sv;
            file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
            file.write( e.data(), static_cast<std::streamsize>( e.size() ) );
            file.write( "}\n", 2 );
          }

          line = R"(\end{description}
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        }

        if ( prop.example.has_value() )
        {
          const auto ex = std::any_cast<std::string>( prop.example );
          line = R"(\item \textbf{Example} - \texttt{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( ex.data(), static_cast<std::streamsize>( ex.size() ) );
          file.write( "}\n", 2 );
        }
        else if ( !prop.examples.empty() )
        {
          auto vec = std::vector<std::string>{};
          vec.reserve( prop.examples.size() );
          for ( const auto& ex : prop.examples )
          {
            if ( !ex.has_value() ) continue;
            vec.emplace_back( std::any_cast<std::string>( ex ) );
          }

          auto v = std::format( "{:n}", vec );
          boost::algorithm::replace_all( v, "\"", "" );
          v = clean( v );
          line = R"(\item \textbf{Examples} - \texttt{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
          file.write( "}\n", 2 );

          if ( !prop.properties.empty() )
          {
            line = R"(\begin{itemize}
)"sv;
            file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

            for ( const auto& [n, p] : prop.properties ) property( p, n );

            line = R"(\end{itemize}
)"sv;
            file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          }
        }

  line = R"(
\end{itemize}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      };

      if ( !schema.properties.empty() )
      {
        line = R"(\begin{itemize}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

        for ( const auto& [name, prop] : schema.properties ) property( prop, name );

        line = R"(\end{itemize}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }
    }

    void writeSchemaAggregations( const spt::model::Schema& schema, std::ofstream& file, bool eol = true )
    {
      const auto process = [&file]( const std::vector<spt::model::Schema>& vector, std::string_view text )
      {
        if ( vector.empty() ) return;

        auto line = R"(\textbf{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( text.data(), static_cast<std::streamsize>( text.size() ) );

        line = R"(}
\begin{itemize}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

        for ( const auto& child : vector )
        {
          const auto title = schemaTitle( child );

          if ( child._referenceURI.empty() )
          {
            writeSchemaForAggregation( child, title, file );
            continue;
          }

          const auto key = referenceKey( child, "schema" );
          line = R"(\item \textbf{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( title.data(), static_cast<std::streamsize>( title.size() ) );
          line = R"(}. See chapter \ref{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( key.data(), static_cast<std::streamsize>( key.size() ) );
          line = R"(} on page \pageref{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( key.data(), static_cast<std::streamsize>( key.size() ) );
          file.write( "}.\n", 3 );
        }

        line = R"(\end{itemize}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      };

      process( schema.oneOf, "One of" );
      process( schema.anyOf, "Any of" );
      process( schema.allOf, "All of" );

      if ( eol )
      {
        auto line = R"(\\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }
    }

    void writeSchema( const spt::model::Parameter& param, std::ofstream& file )
    {
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

    void writeParameter( const spt::model::Parameter& param, std::ofstream& file, bool initial = false )
    {
      if ( !initial )
      {
        auto line = R"(\item \textbf{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( param.name.data(), static_cast<std::streamsize>( param.name.size() ) );
        file.write( "} ", 2 );
      }

      if ( !param.description.empty() )
      {
        const auto desc = spt::output::convert( param.description );
        file.write( desc.data(), static_cast<std::streamsize>( desc.size() ) );
      }

      if ( !initial )
      {
        const auto key = referenceKey( param, "parameter"sv );
        if ( parameterMap.contains( key ) )
        {
          auto line = R"(
See section \ref{)"sv;
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
      }

      auto line = R"(
\begin{description}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      line = R"(\item \textit{in} - )"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      file.write( param.in.data(), static_cast<std::streamsize>( param.in.size() ) );
      line = R"(
\item \textit{required} - )"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      auto str = std::format( "{}", param.required );
      file.write( str.data(), static_cast<std::streamsize>( str.size() ) );
      file.write( "\n", 1 );

      if ( param.deprecated )
      {
        line = R"(\item \textit{deprecated} - true
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( param.allowEmptyValue )
      {
        line = R"(\item \textit{allowEmptyValue} - true
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( param.explode )
      {
        line = R"(\item \textit{explode} - true
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( param.allowReserved )
      {
        line = R"(\item \textit{allowReserved} - true
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( !param.style.empty() )
      {
        line = R"(\item \textit{style} - )"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( param.style.data(), static_cast<std::streamsize>( param.style.size() ) );
        file.write( "\n", 1 );
      }

      if ( param.schema ) writeSchema( param, file );

      line = R"(\end{description}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
    }

    std::string entityTitle( const spt::model::RequestBody& body )
    {
      auto title = body.ref.empty() ? "Request Body"s : body.ref;
      auto parts = spt::util::split( title, 4, "#/" );
      if ( !parts.empty() ) title = parts.back();
      return title;
    }

    void writeMediaType( std::string_view key, const spt::model::MediaType& mt, std::ofstream& file )
    {
      auto line = R"(\textbf{)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      file.write( key.data(), static_cast<std::streamsize>( key.size() ) );
      file.write( "}\n\n", 3 );

      if ( !mt.schema ) return;

      if ( !mt.schema->summary.empty() )
      {
        const auto sum = spt::output::convert( mt.schema->summary );
        file.write( sum.data(), static_cast<std::streamsize>( sum.size() ) );
        file.write( "\n\n", 2 );
      }

      if ( !mt.schema->description.empty() )
      {
        const auto sum = spt::output::convert( mt.schema->description );
        file.write( sum.data(), static_cast<std::streamsize>( sum.size() ) );
        file.write( "\n\n", 2 );
      }

      if ( !mt.schema->_referenceURI.empty() )
      {
        const auto skey = referenceKey( *mt.schema, "schema"sv );
        line = R"(\textbf{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        auto title = schemaTitle( *mt.schema );
        file.write( title.data(), static_cast<std::streamsize>( title.size() ) );
        line = R"(}. See chapter \ref{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( skey.data(), static_cast<std::streamsize>( skey.size() ) );
        line = R"(} on page \pageref{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( skey.data(), static_cast<std::streamsize>( skey.size() ) );
        file.write( "}.\n\n", 4 );
        return;
      }

      if ( mt.schema->properties.empty() ) return;

      line = R"(\begin{itemize}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

      const auto propertySchema = [&file]( std::string name, const spt::model::Schema& schema )
      {
        auto line = R"(\item \textbf{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        auto cleaned = clean( name );
        file.write( cleaned.data(), static_cast<std::streamsize>( cleaned.size() ) );
        line = R"(}
\index{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( cleaned.data(), static_cast<std::streamsize>( cleaned.size() ) );
        file.write( "} ", 2 );

        if ( !schema._referenceURI.empty() )
        {
          const auto skey = referenceKey( schema, "schema"sv );
          line = R"(\textbf{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          const auto title = schemaTitle( schema );
          file.write( title.data(), static_cast<std::streamsize>( title.size() ) );
          line = R"(}. See chapter \ref{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( skey.data(), static_cast<std::streamsize>( skey.size() ) );
          line = R"(} on page \pageref{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( skey.data(), static_cast<std::streamsize>( skey.size() ) );
          line = R"(} for schema.
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        }
        else
        {
          file.write( " - ", 3 );
          file.write( schema.type.data(), static_cast<std::streamsize>( schema.type.size() ) );
          file.write( "\n", 1 );
          schemaExamples( schema, file );
        }
      };

      const auto schemaDetails = [&file, &propertySchema]( std::string name, const spt::model::Schema& schema )
      {
        auto line = R"(\item \textbf{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        auto cleaned = clean( std::move( name ) );
        file.write( cleaned.data(), static_cast<std::streamsize>( cleaned.size() ) );
        line = R"(}
\index{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( cleaned.data(), static_cast<std::streamsize>( cleaned.size() ) );
        file.write( "}\n", 2 );

        if ( !schema._referenceURI.empty() )
        {
          const auto skey = referenceKey( schema, "schema"sv );
          line = R"(\textbf{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          const auto title = schemaTitle( schema );
          file.write( title.data(), static_cast<std::streamsize>( title.size() ) );
          line = R"(}. See chapter \ref{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( skey.data(), static_cast<std::streamsize>( skey.size() ) );
          line = R"(} on page \pageref{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( skey.data(), static_cast<std::streamsize>( skey.size() ) );
          line = R"(} for schema.
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          return;
        }

        if ( !schema.description.empty() )
        {
          const auto desc = spt::output::convert( schema.description );
          file.write( desc.data(), static_cast<std::streamsize>( desc.size() ) );
          file.write( "\n", 1 );
        }

        line = R"(\begin{itemize}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

        line = R"(\item \textbf{Type} )"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( schema.type.data(), static_cast<std::streamsize>( schema.type.size() ) );
        file.write( "\n", 1 );

        if ( !schema.format.empty() )
        {
          line = R"(\item \textbf{Format} )"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( schema.format.data(), static_cast<std::streamsize>( schema.format.size() ) );
          file.write( "\n", 1 );
        }

        if ( !schema.properties.empty() )
        {
          line = R"(\begin{itemize}
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

          for ( const auto& [ppkey, pp] : schema.properties ) propertySchema( ppkey, pp );

          line = R"(\end{itemize}
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        }

        line = R"(\end{itemize}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      };

      for ( const auto& [pname, prop] : mt.schema->properties ) schemaDetails( pname, prop );

      line = R"(\end{itemize}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
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

        for ( const auto& [mkey, mt] : body.content ) writeMediaType( mkey, mt, file );

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
        if ( requestBodyMap.contains( key ) )
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

        requestBodyMap.try_emplace( key, std::cref( body ) );
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

        for ( const auto& param : operation.parameters ) writeParameter( param, file );

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
        const auto title = schemaTitle( *sc.schema );

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
          const auto key = referenceKey( *sc.schema, "schema"sv );
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

        writeSchemaProperties( *sc.schema, file );
        writeSchemaAggregations( *sc.schema, file );
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

    void tags( const std::filesystem::path& parent, const spt::model::OpenAPI& openapi, std::ofstream& mainFile, const spt::model::Configuration& conf )
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
          if ( pi.post ) writeOperation( *pi.post, tag, key, file, conf, "POST"sv );
          if ( pi.put ) writeOperation( *pi.put, tag, key, file, conf, "PUT"sv );
          if ( pi.get ) writeOperation( *pi.get, tag, key, file, conf, "GET"sv );
          if ( pi.patch ) writeOperation( *pi.patch, tag, key, file, conf, "PATCH"sv );
          if ( pi._delete ) writeOperation( *pi._delete, tag, key, file, conf, "DELETE"sv );
          if ( pi.head ) writeOperation( *pi.head, tag, key, file, conf, "HEAD"sv );
          if ( pi.options ) writeOperation( *pi.options, tag, key, file, conf, "OPTIONS"sv );
          if ( pi.trace ) writeOperation( *pi.trace, tag, key, file, conf, "TRACE"sv );
        }

        writeInput( path, mainFile );
      }
    }

    std::expected<std::filesystem::path, std::string> collectParameters( std::filesystem::path path, const spt::model::OpenAPI& openapi )
    {
      using O = std::expected<std::filesystem::path, std::string>;

      const auto collectOperation = []( const spt::model::Operation& operation )
      {
        for ( const auto& param : operation.parameters )
        {
          if ( param._referenceURI.empty() ) continue;
          parameterMap.try_emplace( referenceKey( param, "parameter"sv ), std::cref( param ) );
        }
      };

      if ( openapi.components )
      {
        for ( const auto& [key, param] : openapi.components->parameters )
        {
          if ( param._referenceURI.empty() ) continue;
          parameterMap.try_emplace( referenceKey( param, "parameter"sv ), std::cref( param ) );
        }
      }

      for ( const auto& [_, pi] : openapi.paths )
      {
        for ( const auto& param : pi.parameters )
        {
          if ( param._referenceURI.empty() ) continue;
          parameterMap.try_emplace( referenceKey( param, "parameter"sv ), std::cref( param ) );
        }

        if ( pi.get ) collectOperation( *pi.get );
        if ( pi.post ) collectOperation( *pi.post );
        if ( pi.put ) collectOperation( *pi.put );
        if ( pi._delete ) collectOperation( *pi._delete );
        if ( pi.head ) collectOperation( *pi.head );
        if ( pi.options ) collectOperation( *pi.options );
        if ( pi.trace ) collectOperation( *pi.trace );
      }

      if ( parameterMap.empty() ) return O{ std::unexpect, "No parameters collected" };
      LOG_INFO << "Collected " << static_cast<int>( parameterMap.size() ) << " parameters.";

      path.append( "parameters.tex" );
      auto file = std::ofstream{ path };

      auto line = R"(\chapter{Parameters}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

      for ( const auto& [key, param] : parameterMap )
      {
        line = R"(\section{\label{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( key.data(), static_cast<std::streamsize>( key.size() ) );
        file.write( "}", 1 );
        file.write( param.get().name.data(), static_cast<std::streamsize>( param.get().name.size() ) );
        line = R"(}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        writeParameter( param, file, true );
      }

      file.close();
      return O{ std::in_place, path };
    }

    void writeSchemaInfo( const spt::model::Schema& schema, std::ofstream& file )
    {
      if ( !schema.summary.empty() )
      {
        const auto sum = spt::output::convert( schema.summary );
        auto line = R"(\begin{quote})"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( sum.data(), static_cast<std::streamsize>( sum.size() ) );
        line = R"(\end{quote})"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( "\n", 1 );
      }

      if ( !schema.sinceVersion.empty() )
      {
        auto line = R"(\begin{quote}\textbf{Since Version:} )"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( schema.sinceVersion.data(), static_cast<std::streamsize>( schema.sinceVersion.size() ) );
        line = R"(\end{quote})"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( "\n", 1 );
      }

      if ( !schema.description.empty() )
      {
        const auto sum = spt::output::convert( schema.description );
        file.write( sum.data(), static_cast<std::streamsize>( sum.size() ) );
        file.write( "\n", 1 );
      }
    }

    void writeSchemaExample( const spt::model::Schema& schema, std::ofstream& file )
    {
      if ( schema.examples.empty() ) return;

      auto line = R"(\subsubsection*{Examples}
\begin{lstlisting}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

      for ( const auto& example : schema.examples )
      {
        if ( !example.has_value() ) continue;
        const auto str = std::any_cast<std::string>( example );
        file.write( str.data(), static_cast<std::streamsize>( str.size() ) );
        line = R"(\end{lstlisting}

)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }
    }

    void writeSchemaAggregationsTable( const spt::model::Schema& schema, std::ofstream& file )
    {
      const auto process = [&file]( const std::vector<spt::model::Schema>& vector, std::string_view text )
      {
        if ( vector.empty() ) return;

        auto line = R"(\hline )"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( text.data(), static_cast<std::streamsize>( text.size() ) );

        line = R"( & \begin{itemize}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

        for ( const auto& child : vector )
        {
          const auto title = schemaTitle( child );

          if ( child._referenceURI.empty() )
          {
            /*
            line = R"(\item )"sv;
            file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
            file.write( title.data(), static_cast<std::streamsize>( title.size() ) );
            line = R"( of type )"sv;
            file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
            file.write( child.type.data(), static_cast<std::streamsize>( child.type.size() ) );
            file.write( "\n", 1 );
            */
            writeSchemaForAggregation( child, title, file );
            continue;
          }

          const auto key = referenceKey( child, "schema" );
          line = R"(\item \textbf{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( title.data(), static_cast<std::streamsize>( title.size() ) );
          line = R"(}. See chapter \ref{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( key.data(), static_cast<std::streamsize>( key.size() ) );
          line = R"(} on page \pageref{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( key.data(), static_cast<std::streamsize>( key.size() ) );
          file.write( "}\n", 2 );
        }

        line = R"(\end{itemize} \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      };

      process( schema.oneOf, "One of" );
      process( schema.anyOf, "Any of" );
      process( schema.allOf, "All of" );
    }

    void writeSchema( std::string_view key, const std::string& name, const spt::model::Schema& schema, std::ofstream& file,
      const spt::model::Schema& parent )
    {
      const auto cn = clean( std::string{ name } );
      auto line = R"(\section{\label{)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      file.write( key.data(), static_cast<std::streamsize>( key.size() ) );
      file.write( ":", 1 );
      file.write( cn.data(), static_cast<std::streamsize>( cn.size() ) );
      file.write( "}", 1 );
      file.write( cn.data(), static_cast<std::streamsize>( cn.size() ) );
      file.write( "}\n", 2 );

      writeSchemaInfo( schema, file );

      const auto starttable = [&file, &cn, &parent]
      {
        auto line = R"(\begin{center}
\tablefirsthead{%
  \hline
  \multicolumn{1}{|c}{\textbf{Property}} & \multicolumn{1}{|c|}{\textbf{Value}} \\
  \hline}
\tablehead{%
  \hline
  \multicolumn{2}{|c|}{continued from previous page} \\
  \hline}
\tabletail{%
  \hline
  \multicolumn{2}{|c|}{continued on next page} \\
  \hline
}
\tablelasttail{\hline}
\tablecaption{Properties for )"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        const auto pt = clean(schemaTitle( parent ));
        file.write( pt.data(), static_cast<std::streamsize>( pt.size() ) );
        file.write( "::", 2 );
        file.write( cn.data(), static_cast<std::streamsize>( cn.size() ) );

        line = R"(}
\begin{supertabular}{|l|l|}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      };

      const auto endtable = [&file]
      {
        auto line = R"(
\end{supertabular}
\end{center}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      };

      const auto required = [&parent, &name]
      {
        return std::ranges::find( parent.required, name ) != std::ranges::end( parent.required );
      };

      starttable();

      line = R"(Type & \texttt{)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      file.write( schema.type.data(), static_cast<std::streamsize>( schema.type.size() ) );
      line = R"(} \\
\hline Required & )"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      const auto pr = std::format( "{}", required() );
      file.write( pr.data(), static_cast<std::streamsize>( pr.size() ) );
      line = R"( \\
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

      if ( !schema._referenceURI.empty() )
      {
        const auto rkey = referenceKey( schema, "schema"sv );
        line = R"(\hline Reference & See section \ref{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( rkey.data(), static_cast<std::streamsize>( rkey.size() ) );
        line = R"(} on page \pageref{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( rkey.data(), static_cast<std::streamsize>( rkey.size() ) );
        line = R"(}. \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        endtable();
        return;
      }

      if ( schema.items && !schema.items->_referenceURI.empty() )
      {
        const auto rkey = referenceKey( *schema.items, "schema"sv );
        line = R"(\hline Reference & See section \ref{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( rkey.data(), static_cast<std::streamsize>( rkey.size() ) );
        line = R"(} on page \pageref{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( rkey.data(), static_cast<std::streamsize>( rkey.size() ) );
        line = R"(}. \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( !schema.sinceVersion.empty() )
      {
        line = R"(\hline Since Version & \texttt{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( schema.sinceVersion.data(), static_cast<std::streamsize>( schema.sinceVersion.size() ) );
        line = R"(} \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( schema._default.has_value() )
      {
        line = R"(\hline Default & )"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        const auto def = clean( std::any_cast<std::string>( schema._default ) );
        file.write( def.data(), static_cast<std::streamsize>( def.size() ) );
        line = R"(. \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( !schema.pattern.empty() )
      {
        line = R"(\hline Pattern & \verb|)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( schema.pattern.data(), static_cast<std::streamsize>( schema.pattern.size() ) );
        line = R"(| \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( !schema.format.empty() )
      {
        const auto cleaned = clean( schema.format );
        line = R"(\hline Format & )"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( cleaned.data(), static_cast<std::streamsize>( cleaned.size() ) );
        line = R"(. \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( schema.example.has_value() )
      {
        const auto ex = clean( std::any_cast<std::string>( schema.example ) );
        line = R"(\hline Example & \texttt{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( ex.data(), static_cast<std::streamsize>( ex.size() ) );
        line = R"(}. \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }
      else if ( schema.examples.size() == 1 )
      {
        const auto& example = schema.examples.front();
        if ( example.has_value() )
        {
          const auto ex = clean( std::any_cast<std::string>( example ) );
          line = R"(\hline Example & \texttt{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( ex.data(), static_cast<std::streamsize>( ex.size() ) );
          line = R"(}. \\
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        }
      }

      if ( schema.maximum.has_value() )
      {
        const auto v = std::format( "{}", *schema.maximum );
        line = R"(\hline Maximum & \texttt{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
        line = R"(}. \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( schema.exclusiveMaximum.has_value() )
      {
        const auto v = std::format( "{}", *schema.exclusiveMaximum );
        line = R"(\hline Exclusive Maximum & \texttt{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
        line = R"(}. \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( schema.minimum.has_value() )
      {
        const auto v = std::format( "{}", *schema.minimum );
        line = R"(\hline Minimum & \texttt{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
        line = R"(}. \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( schema.exclusiveMinimum.has_value() )
      {
        const auto v = std::format( "{}", *schema.exclusiveMinimum );
        line = R"(\hline Exclusive Minimum & \texttt{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
        line = R"(}. \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( schema.maxLength.has_value() )
      {
        const auto v = std::format( "{}", *schema.maxLength );
        line = R"(\hline Max Length & \texttt{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
        line = R"(}. \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( schema.minLength.has_value() )
      {
        const auto v = std::format( "{}", *schema.minLength );
        line = R"(\hline Min Length & \texttt{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
        line = R"(}. \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( schema.maxItems.has_value() )
      {
        const auto v = std::format( "{}", *schema.maxItems );
        line = R"(\hline Max Items & \texttt{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
        line = R"(}. \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( schema.minItems.has_value() )
      {
        const auto v = std::format( "{}", *schema.minItems );
        line = R"(\hline Min Items & \texttt{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
        line = R"(}. \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( !schema.enumeration.empty() )
      {
        auto v = std::format( "{:n}", schema.enumeration );
        boost::algorithm::replace_all( v, "\"", "" );
        line = R"(\hline Enum & Allowed values - \texttt{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
        line = R"(}. \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( schema.nullable )
      {
        const auto v = std::format( "{}", schema.nullable );
        line = R"(\hline Nullable & \texttt{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
        line = R"(}. \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( schema.readOnly )
      {
        const auto v = std::format( "{}", schema.readOnly );
        line = R"(\hline Read Only & \texttt{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
        line = R"(}. \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( schema.writeOnly )
      {
        const auto v = std::format( "{}", schema.writeOnly );
        line = R"(\hline Write Only & \texttt{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
        line = R"(}. \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( schema.deprecated )
      {
        const auto v = std::format( "{}", schema.deprecated );
        line = R"(\hline Deprecated & \texttt{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
        line = R"(}. \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      writeSchemaAggregationsTable( schema, file );
      endtable();

      if ( schema.examples.size() > 1 && !schema.example.has_value() ) writeSchemaExample( schema, file );

      for ( const auto& [pname, prop] : schema.properties )
      {
        if ( !prop._referenceURI.empty() ) continue;
        writeSchema( referenceKey( prop, "schema"sv ), pname, prop, file, schema );
      }

      if ( schema.items && !schema.items->_referenceURI.empty() )
      {
        writeSchema( referenceKey( *schema.items, "schema"sv ), schema.items->title, *schema.items, file, schema );
      }
    }

    std::filesystem::path writeSchema( std::string_view key, const spt::model::Schema& schema, std::filesystem::path path )
    {
      const auto pos = schema._referenceURI.find( '#' );
      auto fn = pos == std::string::npos ? schema._referenceURI : schema._referenceURI.substr( 0, pos );
      auto fp = std::filesystem::path{ fn };
      const auto title = schemaTitle( schema );

      path.append( std::format( "schema-{}-{}.tex", fp.stem().string(), title) );
      LOG_DEBUG << "Writing schema with reference " << key << " to file " << path.string();
      auto file = std::ofstream{ path };

      auto line = R"(
\chapter{\label{)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      file.write( key.data(), static_cast<std::streamsize>( key.size() ) );
      file.write( "}", 1 );
      file.write( title.data(), static_cast<std::streamsize>( title.size() ) );
      file.write( "}\n", 2 );

      writeSchemaInfo( schema, file );

      for ( const auto& [name, sc] : schema.properties ) writeSchema( key, name, sc, file, schema );

      writeSchemaAggregations( schema, file, false );

      file.close();
      return path;
    }

    struct SampleWrapper
    {
      std::vector<std::reference_wrapper<const spt::model::CodeSample>> samples;
      std::string operationId;
    };

    std::filesystem::path codeSamples( std::string_view tag, const std::vector<SampleWrapper>& vector, std::filesystem::path path )
    {
      path.append( std::format( "codesamples-{}.tex", tag ) );
      auto file = std::ofstream{ path };

      auto line = R"(\chapter{\label{codesamples:)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      file.write( tag.data(), static_cast<std::streamsize>( tag.size() ) );
      file.write( "}", 1 );
      file.write( tag.data(), static_cast<std::streamsize>( tag.size() ) );
      file.write( "}\n", 2 );

      for ( const auto& wrapper : vector )
      {
        line = R"(\section{\label{codesamples:)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( wrapper.operationId.data(), static_cast<std::streamsize>( wrapper.operationId.size() ) );
        file.write( "}", 1 );
        file.write( wrapper.operationId.data(), static_cast<std::streamsize>( wrapper.operationId.size() ) );
        file.write( "}\n", 2 );

        for ( const auto& cs : wrapper.samples )
        {
          auto key = cs.get().label.empty() ? cs.get().lang : cs.get().label;
          line = R"(\subsection{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( key.data(), static_cast<std::streamsize>( key.size() ) );
          file.write( "}\n", 2 );
          line = R"(\begin{lstlisting}
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( cs.get().source.data(), static_cast<std::streamsize>( cs.get().source.size() ) );
          line = R"(
\end{lstlisting}
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        }
      }

      return path;
    }

    std::expected<std::filesystem::path, std::string> codeSamples( const spt::model::OpenAPI& openapi, std::filesystem::path path )
    {
      using O = std::expected<std::filesystem::path, std::string>;
      auto map = std::map<std::string, std::vector<SampleWrapper>>{};

      const auto tagNameOperation = []( const spt::model::Operation& operation, const std::string& tag ) -> bool
      {
        return std::ranges::find( operation.tags, tag ) != std::ranges::end( operation.tags );
      };

      const auto tagName = [&openapi, &tagNameOperation]( const spt::model::PathItem& pi ) -> std::string
      {
        for ( const auto& tag : openapi.tags )
        {
          if ( pi.get && tagNameOperation( *pi.get, tag.name ) ) return tag.name;
          if ( pi.post && tagNameOperation( *pi.post, tag.name ) ) return tag.name;
          if ( pi.put && tagNameOperation( *pi.put, tag.name ) ) return tag.name;
          if ( pi._delete && tagNameOperation( *pi._delete, tag.name ) ) return tag.name;
          if ( pi.patch && tagNameOperation( *pi.patch, tag.name ) ) return tag.name;
          if ( pi.head && tagNameOperation( *pi.head, tag.name ) ) return tag.name;
          if ( pi.options && tagNameOperation( *pi.options, tag.name ) ) return tag.name;
          if ( pi.trace && tagNameOperation( *pi.trace, tag.name ) ) return tag.name;
        }

        return {};
      };

      const auto add = [&map]( const spt::model::Operation& operation, const std::string& tag )
      {
        if ( operation.codeSamples.empty() ) return;

        map.try_emplace( tag );
        auto& vec = map.at( tag );
        auto& wrapper = vec.emplace_back();

        wrapper.operationId = operation.operationId;
        wrapper.samples.reserve( operation.codeSamples.size() );
        for ( const auto& cs : operation.codeSamples ) wrapper.samples.emplace_back( std::cref( cs ) );
      };

      for ( const auto& [_, pi] : openapi.paths )
      {
        auto tn = tagName( pi );
        if ( tn.empty() ) continue;

        if ( pi.get && !pi.get->codeSamples.empty() ) add( *pi.get, tn );
        if ( pi.post && !pi.post->codeSamples.empty() ) add( *pi.post, tn );
        if ( pi.put && !pi.put->codeSamples.empty() ) add( *pi.put, tn );
        if ( pi._delete && !pi._delete->codeSamples.empty() ) add( *pi._delete, tn );
        if ( pi.patch && !pi.patch->codeSamples.empty() ) add( *pi.patch, tn );
        if ( pi.head && !pi.head->codeSamples.empty() ) add( *pi.head, tn );
        if ( pi.options && !pi.options->codeSamples.empty() ) add( *pi.options, tn );
        if ( pi.trace && !pi.trace->codeSamples.empty() ) add( *pi.trace, tn );
      }

      if ( map.empty() ) return O{ std::unexpect, "No code samples" };

      path.append( "codesamples.tex" );
      auto file = std::ofstream{ path };

      auto line = R"(\part{Code Samples}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

      for ( const auto& [tag, samples] : map )
      {
        auto genpath = codeSamples( tag, samples, path.parent_path() );
        writeInput( genpath, file );
      }

      file.close();
      return O{ std::in_place, path };
    }

    using SchemaMap = std::map<std::string, std::reference_wrapper<const spt::model::Schema>, std::less<>>;
    void schemasFromOperation( const spt::model::Operation& op, SchemaMap& map );
    void schemasFromPathItem( const spt::model::PathItem& pi, SchemaMap& map );

    void nestedSchemas( const spt::model::Schema& schema, SchemaMap& map )
    {
      if ( !schema._referenceURI.empty() ) map.try_emplace( referenceKey( schema, "schema"sv ), std::cref( schema ) );

      if ( schema.items ) nestedSchemas( *schema.items, map );
      for ( const auto& prop : schema.properties ) nestedSchemas( prop.second, map );
      for ( const auto& child : schema.anyOf ) nestedSchemas( child, map );
      for ( const auto& child : schema.oneOf ) nestedSchemas( child, map );
      for ( const auto& child : schema.allOf ) nestedSchemas( child, map );
    }

    void schemasFromOperation( const spt::model::Operation& op, SchemaMap& map )
    {
      const auto add = [&map]( const spt::model::MediaType& mt )
      {
        if ( !mt.schema ) return;
        nestedSchemas( *mt.schema, map );
      };

      for ( const auto& param : op.parameters )
      {
        if ( param.schema && !param.schema->_referenceURI.empty() ) nestedSchemas( *param.schema, map );
      }

      for ( const auto& [_, resp] : op.responses )
      {
        for ( const auto& [_m, mt] : resp.content ) add( mt );
      }

      for ( const auto& [_, pi] : op.callbacks ) schemasFromPathItem( pi, map );
      if ( op.requestBody )
      {
        for ( const auto& [_, mt] : op.requestBody->content ) add( mt );
      }
    }

    void schemasFromPathItem( const spt::model::PathItem& pi, SchemaMap& map )
    {
      for ( const auto& param : pi.parameters )
      {
        if ( param.schema && !param.schema->_referenceURI.empty() ) nestedSchemas( *param.schema, map );
      }

      if ( pi.get ) schemasFromOperation( *pi.get, map );
      if ( pi.post ) schemasFromOperation( *pi.post, map );
      if ( pi.put ) schemasFromOperation( *pi.put, map );
      if ( pi._delete ) schemasFromOperation( *pi._delete, map );
      if ( pi.patch ) schemasFromOperation( *pi.patch, map );
      if ( pi.head ) schemasFromOperation( *pi.head, map );
      if ( pi.options ) schemasFromOperation( *pi.options, map );
      if ( pi.trace ) schemasFromOperation( *pi.trace, map );
    }

    SchemaMap collectSchemas( const spt::model::OpenAPI& openapi )
    {
      auto map = SchemaMap{};
      for ( const auto& [_, pi] : openapi.paths ) schemasFromPathItem( pi, map );
      return map;
    }
  }
}

std::string spt::output::generate( model::OpenAPI& openapi, const model::Configuration& config )
{
  auto p = std::filesystem::weakly_canonical( std::filesystem::path( config.output ) ).make_preferred();

  if ( !std::filesystem::is_directory( p ) )
  {
    if ( !std::filesystem::create_directories( p ) )
    {
      LOG_CRIT << "Error creating directory " << config.output;
      return "";
    }
  }

  auto outfile = p;
  outfile.append( "openapi.tex" );

  auto file = std::ofstream{ outfile };

  auto genpath = poutput::preamble( p );
  poutput::writeInput( genpath, file );

  genpath = poutput::frontmatter( p, openapi, config );
  poutput::writeInput( genpath, file );

  genpath = poutput::info( p, openapi, config.cmark );
  poutput::writeInput( genpath, file );

  poutput::servers( openapi, file );
  if ( openapi.components ) poutput::securitySchemes( *openapi.components, file );
  if ( openapi.components ) poutput::examples( *openapi.components, file );

  if ( auto cps = poutput::collectParameters( p, openapi ); cps.has_value() ) poutput::writeInput( *cps, file );

  auto line = R"(\part{Endpoints})"sv;
  file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

  if ( const auto tgs = poutput::tagGroups( p, openapi ); tgs.has_value() ) poutput::writeInput( *tgs, file );
  poutput::tags( p, openapi, file, config );

  auto map = poutput::collectSchemas( openapi );
  LOG_INFO << "Gathered " << int(map.size() ) << " schemas.";
  if ( !map.empty() )
  {
    line = R"(\part{Schemas}
)"sv;
    file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

    using Pair = std::pair<std::string, std::reference_wrapper<const model::Schema>>;
    auto vec = std::vector<Pair>{};
    vec.reserve( map.size() );
    for ( const auto& [key, schema] : map ) vec.emplace_back( key, schema );

    std::ranges::stable_sort( vec, []( const Pair& lhs, const Pair& rhs )
    {
      if ( !lhs.second.get()._referenceURI.empty() && !rhs.second.get()._referenceURI.empty() )
      {
        return lhs.second.get()._referenceURI < rhs.second.get()._referenceURI;
      }

      return lhs.second.get().title < rhs.second.get().title;
    } );

    for ( const auto& [key, schema] : vec )
    {
      genpath = poutput::writeSchema( key, schema, p );
      poutput::writeInput( genpath, file );
    }
  }

  if ( const auto cs = poutput::codeSamples( openapi, p ); cs.has_value() )
  {
    poutput::writeInput( *cs, file );
  }

  line = R"(
\backmatter
\listoftables
\clearpage
\printindex % Print the index at the very end of the document
\end{document})"sv;
  file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

  file.close();

  return outfile.string();
}