//
// Created by Rakesh on 22/09/2025.
//

#include "output.hpp"
#include "log/NanoLog.hpp"

#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
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

  auto back = R"(
\backmatter
\listoftables
\clearpage
\printindex % Print the index at the very end of the document
\end{document})"sv;
  file.write( back.data(), static_cast<std::streamsize>( back.size() ) );

  file.close();

  return outfile.string();
}