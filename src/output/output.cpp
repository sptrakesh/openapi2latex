//
// Created by Rakesh on 22/09/2025.
//

#include "output.hpp"
#include "log/NanoLog.hpp"
#include "impl/collectors.hpp"
#include "impl/output.hpp"
#include "util/split.hpp"

#include <cstdlib>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <ranges>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/case_conv.hpp>
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
    std::filesystem::path preamble( std::filesystem::path path, const spt::model::Configuration& conf )
    {
      path.append( "preamble.tex" );
      auto file = std::ofstream{ path };

      if ( conf.font == "Helvetica Neue" )
      {
        file.write( preambleContents.data(), preambleContents.size() );
      }
      else
      {
        auto data = std::string{ preambleContents };
        boost::algorithm::replace_all( data, "Helvetica Neue", conf.font );
        file.write( data.data(), static_cast<std::streamsize>( data.size() ) );
      }

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
        const auto ckey = spt::output::impl::clean( key );
        file.write( ckey.data(), static_cast<std::streamsize>( ckey.size() ) );

        line = R"(}} & Type & )"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        const auto stype = spt::output::impl::clean( scheme.type );
        file.write( stype.data(), static_cast<std::streamsize>( stype.size() ) );
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
          auto desc = spt::output::impl::clean( scheme.name );
          file.write( desc.data(), static_cast<std::streamsize>( desc.size() ) );
          line = R"( \\
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        }

        if ( !scheme.in.empty() )
        {
          line = R"(\cline{2-3} & In & )"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          auto desc = spt::output::impl::clean( scheme.in );
          file.write( desc.data(), static_cast<std::streamsize>( desc.size() ) );
          line = R"( \\
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        }

        if ( !scheme.scheme.empty() )
        {
          line = R"(\cline{2-3} & Scheme & )"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          auto desc = spt::output::impl::clean( scheme.scheme );
          file.write( desc.data(), static_cast<std::streamsize>( desc.size() ) );
          line = R"( \\
)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        }

        if ( !scheme.bearerFormat.empty() )
        {
          line = R"(\cline{2-3} & Bearer Format & )"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          auto desc = spt::output::impl::clean( scheme.bearerFormat );
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

    std::expected<std::filesystem::path, std::string> examples( const spt::model::Components& components, std::filesystem::path path )
    {
      using O = std::expected<std::filesystem::path, std::string>;
      if ( components.examples.empty() ) return O{ std::unexpect, "No examples" };

      path.append( "component-examples.tex" );
      auto file = std::ofstream{ path };

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

      return O{ std::in_place, path };
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

    std::expected<std::filesystem::path, std::string> collectParameters( std::filesystem::path path, const spt::model::OpenAPI& openapi )
    {
      using O = std::expected<std::filesystem::path, std::string>;

      const auto collectOperation = []( const spt::model::Operation& operation )
      {
        for ( const auto& param : operation.parameters )
        {
          if ( param._referenceURI.empty() ) continue;
          spt::output::impl::Collectors::instance().parameterMap.try_emplace( spt::output::impl::referenceKey( param, "parameter"sv ), std::cref( param ) );
        }
      };

      if ( openapi.components )
      {
        for ( const auto& [key, param] : openapi.components->parameters )
        {
          if ( param._referenceURI.empty() ) continue;
          spt::output::impl::Collectors::instance().parameterMap.try_emplace( spt::output::impl::referenceKey( param, "parameter"sv ), std::cref( param ) );
        }
      }

      for ( const auto& [_, pi] : openapi.paths )
      {
        for ( const auto& param : pi.parameters )
        {
          if ( param._referenceURI.empty() ) continue;
          spt::output::impl::Collectors::instance().parameterMap.try_emplace( spt::output::impl::referenceKey( param, "parameter"sv ), std::cref( param ) );
        }

        if ( pi.get ) collectOperation( *pi.get );
        if ( pi.post ) collectOperation( *pi.post );
        if ( pi.put ) collectOperation( *pi.put );
        if ( pi._delete ) collectOperation( *pi._delete );
        if ( pi.head ) collectOperation( *pi.head );
        if ( pi.options ) collectOperation( *pi.options );
        if ( pi.trace ) collectOperation( *pi.trace );
      }

      if ( spt::output::impl::Collectors::instance().parameterMap.empty() ) return O{ std::unexpect, "No parameters collected" };
      LOG_INFO << "Collected " << static_cast<int>( spt::output::impl::Collectors::instance().parameterMap.size() ) << " parameters.";

      path.append( "parameters.tex" );
      auto file = std::ofstream{ path };

      auto line = R"(\chapter{Parameters}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

      for ( const auto& [key, param] : spt::output::impl::Collectors::instance().parameterMap )
      {
        line = R"(\section{\label{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( key.data(), static_cast<std::streamsize>( key.size() ) );
        file.write( "}", 1 );
        file.write( param.get().name.data(), static_cast<std::streamsize>( param.get().name.size() ) );
        line = R"(}
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        spt::output::impl::writeParameter( param, file, true );
      }

      file.close();
      return O{ std::in_place, path };
    }

    struct SampleWrapper
    {
      std::vector<std::reference_wrapper<const spt::model::CodeSample>> samples;
      std::string operationId;
    };

    std::filesystem::path codeSamples( std::string_view tag, const std::vector<SampleWrapper>& vector, std::filesystem::path path )
    {
      static const auto languages = std::array{ "C++"s, "Go"s, "Java"s, "Python"s };
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
          line = R"(\begin{lstlisting})"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          if ( const auto iter = std::ranges::find( languages, cs.get().lang ); iter != std::ranges::end( languages ) )
          {
            line = R"([language=)"sv;
            file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
            file.write( cs.get().lang.data(), static_cast<std::streamsize>( cs.get().lang.size() ) );
            file.write( "]\n", 2 );
          }
          else file.write( "\n", 1 );
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
        spt::output::impl::writeInput( genpath, file );
      }

      file.close();
      return O{ std::in_place, path };
    }

    using SchemaMap = std::map<std::string, std::reference_wrapper<const spt::model::Schema>, std::less<>>;
    void schemasFromOperation( const spt::model::Operation& op, SchemaMap& map );
    void schemasFromPathItem( const spt::model::PathItem& pi, SchemaMap& map );

    void nestedSchemas( const spt::model::Schema& schema, SchemaMap& map )
    {
      if ( !schema._referenceURI.empty() ) map.try_emplace( spt::output::impl::referenceKey( schema, "schema"sv ), std::cref( schema ) );

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
      if ( openapi.components )
      {
        for ( const auto& [_, schema] : openapi.components->schemas ) nestedSchemas( schema, map );
      }
      for ( const auto& [_, pi] : openapi.paths ) schemasFromPathItem( pi, map );
      return map;
    }

    std::filesystem::path writeRequestBody( const spt::model::RequestBody& body, std::string_view key, std::filesystem::path path )
    {
      static int counter = 0;
      path.append( std::format( "requestbody-{}.tex", ++counter ) );
      auto file = std::ofstream{ path };

      auto line = R"(\chapter{\label{)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      file.write( key.data(), static_cast<std::streamsize>( key.size() ) );
      line = R"(}Request Body}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

      if ( !body.description.empty() )
      {
        auto desc = spt::output::convert( body.description );
        file.write( desc.data(), static_cast<std::streamsize>( desc.size() ) );
        file.write( "\n", 1 );
      }

      for ( const auto& [name, mt] : body.content )
      {
        spt::output::impl::writeMediaType( name, mt, file );
        file.write( "\n", 1 );
      }

      file.close();
      return  path;
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

  auto genpath = poutput::preamble( p, config );
  impl::writeInput( genpath, file );

  genpath = poutput::frontmatter( p, openapi, config );
  impl::writeInput( genpath, file );

  genpath = poutput::info( p, openapi, config.cmark );
  impl::writeInput( genpath, file );

  poutput::servers( openapi, file );
  if ( openapi.components ) poutput::securitySchemes( *openapi.components, file );
  if ( openapi.components )
  {
    auto ex = poutput::examples( *openapi.components, p );
    if ( ex.has_value() ) impl::writeInput( ex.value(), file );
  }

  if ( auto cps = poutput::collectParameters( p, openapi ); cps.has_value() ) impl::writeInput( *cps, file );

  auto line = R"(\part{Endpoints})"sv;
  file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

  if ( const auto tgs = poutput::tagGroups( p, openapi ); tgs.has_value() ) impl::writeInput( *tgs, file );
  impl::tags( p, openapi, file, config );

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
      genpath = impl::writeSchema( key, schema, p );
      impl::writeInput( genpath, file );
    }
  }

  if ( const auto responses = impl::responses( openapi, p ); responses.has_value() )
  {
    impl::writeInput( responses.value(), file );
  }

  if ( !impl::Collectors::instance().requestBodyMap.empty() )
  {
    LOG_INFO << "Collected " << int(impl::Collectors::instance().requestBodyMap.size()) << " request body references.";

    line = R"(\part{Request Bodies}
)"sv;
    file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

    for ( const auto& [key, body] : impl::Collectors::instance().requestBodyMap )
    {
      genpath = poutput::writeRequestBody( body, key, p );
      impl::writeInput( genpath, file );
    }
  }

  if ( const auto cs = poutput::codeSamples( openapi, p ); cs.has_value() )
  {
    impl::writeInput( *cs, file );
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