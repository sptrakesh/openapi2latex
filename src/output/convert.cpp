//
// Created by Rakesh on 22/09/2025.
//

#include "output.hpp"
#include "../util/split.hpp"

#include <regex>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>

using std::operator ""sv;

namespace
{
  namespace pconvert
  {
    std::string heading( std::string_view data )
    {
      if ( !data.contains( '#' ) ) return std::string{ data };

      auto result = std::string{};
      result.reserve( data.size() );
      auto parts = std::vector<std::string>{};
      parts.reserve( 8 );
      boost::algorithm::split( parts, data, boost::is_any_of( "\r\n" ) );
      auto first = true;

      for ( const auto& line : parts )
      {
        if ( !first ) result.append( "\n" );
        if ( line.empty() ) result.append( "\n" );
        else if ( line.starts_with( "# " ) ) result.append( R"(\section{)"sv ).append( line.substr( 2 ) ).append( "}\n" );
        else if ( line.starts_with( "## " ) ) result.append( R"(\section{)"sv ).append( line.substr( 3 ) ).append( "}\n" );
        else if ( line.starts_with( "### " ) ) result.append( R"(\section{)"sv ).append( line.substr( 4 ) ).append( "}\n" );
        else if ( line.starts_with( "#### " ) ) result.append( R"(\subsection{)"sv ).append( line.substr( 5 ) ).append( "}\n" );
        else if ( line.starts_with( "##### " ) ) result.append( R"(\subsubsection{)"sv ).append( line.substr( 6 ) ).append( "}\n" );
        else result.append( line );
        first = false;
      }

      return result;
    }

    std::string listing( std::string_view data )
    {
      if ( !data.contains( "```" ) ) return std::string{ data };

      auto block = false;
      auto result = std::string{};
      result.reserve( data.size() );
      auto parts = std::vector<std::string>{};
      parts.reserve( 8 );
      boost::algorithm::split( parts, data, boost::is_any_of( "\r\n" ) );
      auto first = true;

      for ( const auto& line : parts )
      {
        if ( !first ) result.append( "\n" );

        if ( line.starts_with( "```" ) )
        {
          if ( block )
          {
            result.append( R"(\end{lstlisting}
)"sv );
            block = false;
          }
          else
          {
            result.append( R"(\begin{lstlisting}
)"sv );
            block = true;
          }
        }
        else result.append( line );
        first = false;
      }

      return result;
    }

    std::string bullets( const std::string& data )
    {
      auto block = false;
      std::size_t indent = 1;
      std::size_t numindents = 0;

      auto result = std::string{};
      result.reserve( data.size() );
      auto parts = std::vector<std::string>{};
      parts.reserve( 8 );
      boost::algorithm::split( parts, data, boost::is_any_of( "\r\n" ) );
      auto first = true;

      auto asterix = std::regex( "^\\s*\\* " );
      auto dash = std::regex( "^\\s*- " );
      for ( const auto& line : parts )
      {
        if ( !first ) result.append( "\n" );
        if ( std::regex_search( line, asterix ) )
        {
          auto idx = line.find( '*' );
          if ( !block )
          {
            result.append( R"(\begin{itemize}
)"sv );
            block = true;
          }
          else if ( idx > indent )
          {
            result.append( R"(\begin{itemize}
)"sv );
            ++numindents;
          }
          else if ( idx < indent )
          {
            result.append( R"(\end{itemize}
)"sv );
            --numindents;
          }

          indent = idx;
          result.append( R"(\item )" ).append( line.substr( idx + 2 ) );
        }
        else if ( std::regex_search( line, dash ) )
        {
          auto idx = line.find( '-' );
          if ( !block )
          {
            result.append( R"(\begin{itemize}
)"sv );
            block = true;
          }
          result.append( R"(\item )"sv ).append( line.substr( idx + 2 ) );
        }
        else if ( block && line.empty() )
        {
          block = false;
          result.append( R"(\end{itemize}
)"sv );
          while ( numindents > 0 )
          {
            result.append( R"(\end{itemize}
)"sv );
            --numindents;
          }
        }
        else result.append( line );

        first = false;
      }

      return result;
    }

    std::string link( const std::string& data )
    {
      auto bs = data.find( '[' );
      if ( bs == std::string::npos ) return data;

      auto result = std::string{};
      result.reserve( data.size() );
      std::size_t pos = 0;

      while ( bs != std::string::npos )
      {
        result.append( data.substr( pos, bs - pos ) );

        auto be = data.find( "](", bs );
        if ( be == std::string::npos ) return data;
        auto e = data.find( ')', be );
        if ( e == std::string::npos ) return data;

        result.append( R"(\href{)" ).
          append( data.substr( be + 2, e - be - 2 ) ).append( "}{" ).
          append( data.substr( bs + 1, be - bs - 1 ) ).append( "}" );

        pos = e + 1;
        bs = data.find( '[', e );
      }

      if ( pos < result.size() ) result.append( data.substr( pos ) );

      return result;
    }

    std::string style( std::string_view data, std::string_view delimiter, std::string_view cmd )
    {
      auto idx = data.find( delimiter );
      if ( idx == std::string_view::npos ) return std::string{ data };

      auto result = std::string{};
      result.reserve( data.size() );

      auto start = idx + delimiter.size();
      auto end = data.find( delimiter, start + 1 );
      if ( end == std::string_view::npos ) return std::string{ data };

      if ( idx > 0 ) result.append( data.substr( 0, idx ) );
      result.append( cmd ).append( "{" ).
        append( data.substr( start, end - start ) ).append( "}" );

      if ( const auto e = end + delimiter.size(); e < data.size() ) result.append( data.substr( e ) );

      return style( result, delimiter, cmd );
    }

    std::string style( std::string_view data )
    {
      auto result = std::string{};
      result.reserve( data.size() );
      auto parts = std::vector<std::string>{};
      parts.reserve( 8 );
      boost::algorithm::split( parts, data, boost::is_any_of( "\r\n" ) );
      auto first = true;

      for ( const auto& line : parts )
      {
        if ( !first ) result.append( "\n" );
        if ( line.empty() ) continue;

        auto processed = style( line, "`"sv, R"(\texttt)"sv );
        processed = style( processed, "**"sv, R"(\textbf)"sv );
        processed = style( processed, "__"sv, R"(\textbf)"sv );
        processed = style( processed, "*"sv, R"(\textit)"sv );
        processed = style( processed, "_"sv, R"(\textit)"sv );
        boost::algorithm::replace_all( processed, "_", R"(\textunderscore )" );

        result.append( processed );
        first = false;
      }

      return result;
    }
  }
}

std::string spt::output::convert( std::string_view data )
{
  auto contents = std::string{ data };
  boost::algorithm::replace_all( contents, "$", R"(\$)" );
  contents = pconvert::heading( contents );
  contents = pconvert::listing( contents );
  contents = pconvert::bullets( contents );
  contents = pconvert::link( contents );
  contents = pconvert::style( contents );
  boost::algorithm::replace_all( contents, "#", R"(\#)" );
  boost::algorithm::replace_all( contents, "%", R"(\%)" );
  return contents;
}