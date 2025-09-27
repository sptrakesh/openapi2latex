//
// Created by Rakesh on 25/09/2025.
//

#include "output.hpp"
#include "log/NanoLog.hpp"
#include "util/split.hpp"

#if defined(__unix__) && !defined(__APPLE__)
#include <fmt/format.h>
#include <fmt/ranges.h>
#else
#include <format>
#endif

using std::operator ""s;
using std::operator ""sv;

namespace
{
  namespace pschema
  {
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
          const auto title = spt::output::impl::schemaTitle( child );

          if ( child._referenceURI.empty() )
          {
            spt::output::impl::writeSchemaForAggregation( child, title, file );
            continue;
          }

          const auto key = spt::output::impl::referenceKey( child, "schema"sv );
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

    void writeSchema( const std::string& name, const spt::model::Schema& schema, std::ofstream& file,
      const spt::model::Schema& parent, std::size_t level )
    {
      static const auto levels = std::array{ "section"sv, "subsection"sv, "subsubsection"sv, "paragraph"sv, "subparagraph"sv };

      const auto cn = spt::output::impl::clean( std::string{ name } );
      LOG_DEBUG << "Writing child " << schema.title << " for parent " << parent.title;
      file.write( R"(\)", 1 );
      auto l = level < levels.size() ? levels[ level ] : "subparagraph"sv;
      file.write( l.data(), static_cast<std::streamsize>( l.size() ) );
      file.write( "{", 1 );
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
        const auto pt = spt::output::impl::clean( spt::output::impl::schemaTitle( parent ) );
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

      auto line = R"(Type & \texttt{)"sv;
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
        const auto rkey = spt::output::impl::referenceKey( schema, "schema"sv );
        line = R"(\hline Reference & See chapter \ref{)"sv;
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

      if ( schema.items )
      {
        if ( !schema.items->_referenceURI.empty() )
        {
          const auto rkey = spt::output::impl::referenceKey( *schema.items, "schema"sv );
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
        else
        {
          const auto title = schema.items->title.empty() ? "Schema" : schema.items->title;
          line = R"(\hline \textbf{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( title.data(), static_cast<std::streamsize>( title.size() ) );
          file.write( "}\n", 2 );

          spt::output::impl::writeSummary( *schema.items, file );
          spt::output::impl::writeDescription( *schema.items, file );
          spt::output::impl::writeSchemaProperties( *schema.items, file );
          line = R"( \\
  )"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        }
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
        const auto def = spt::output::impl::clean( std::any_cast<std::string>( schema._default ) );
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
        const auto cleaned = spt::output::impl::clean( schema.format );
        line = R"(\hline Format & )"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( cleaned.data(), static_cast<std::streamsize>( cleaned.size() ) );
        line = R"(. \\
)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      if ( schema.example.has_value() )
      {
        const auto ex = spt::output::impl::clean( std::any_cast<std::string>( schema.example ) );
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
          const auto ex = spt::output::impl::clean( std::any_cast<std::string>( example ) );
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
#if defined(__unix__) && !defined(__APPLE__)
        auto v = fmt::format( "{:n}", schema.enumeration );
#else
        auto v = std::format( "{:n}", schema.enumeration );
#endif
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
        writeSchema( pname, prop, file, schema, ++level );
      }
    }
  }
}

std::string spt::output::impl::schemaTitle( const model::Schema& schema )
{
  if ( !schema.title.empty() ) return schema.title;
  if ( schema.ref.empty() ) return "Schema"s;
  auto parts = util::split( schema.ref, 4, "#/"sv );
  return parts.empty() ? "Schema"s : std::string{ parts.back() };
}

void spt::output::impl::schemaExamples( const model::Schema& schema, std::ofstream& file )
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

void spt::output::impl::writeSchemaForAggregation( const model::Schema& schema, std::string_view title, std::ofstream& file )
{
  auto line = R"(\item )"sv;
  file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
  file.write( title.data(), static_cast<std::streamsize>( title.size() ) );
  line = R"( of type )"sv;
  file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
  file.write( schema.type.data(), static_cast<std::streamsize>( schema.type.size() ) );
  file.write( "\n", 1 );

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

  writeSummary( schema, file );
  writeDescription( schema, file );

  const std::function<void( const model::Schema& prop, const std::string& name )> property = [&file, &cleanedProperty, &boolean, &optdouble, &property]( const spt::model::Schema& prop, const std::string& name )
  {
    auto line = R"(\item \textbf{)"sv;
    file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
    file.write( name.data(), static_cast<std::streamsize>( name.size() ) );
    line = R"(}
\index{)"sv;
    file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
    file.write( name.data(), static_cast<std::streamsize>( name.size() ) );
    file.write( "}\n", 2 );

    writeSummary( prop, file );

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

    writeDescription( prop, file );

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

#if defined(__unix__) && !defined(__APPLE__)
      auto v = fmt::format( "{:n}", vec );
#else
      auto v = std::format( "{:n}", vec );
#endif
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

void spt::output::impl::writeSchemaProperties( const model::Schema& schema, std::ofstream& file )
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
      const auto desc = convert( schema.description );
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

void spt::output::impl::writeSchemaAggregations( const model::Schema& schema, std::ofstream& file, bool eol )
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

std::filesystem::path spt::output::impl::writeSchema( std::string_view key, const model::Schema& schema, std::filesystem::path path )
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

  pschema::writeSchemaInfo( schema, file );

  for ( const auto& [name, sc] : schema.properties ) pschema::writeSchema( name, sc, file, schema, 0 );

  writeSchemaAggregations( schema, file, false );

  file.close();
  return path;
}
