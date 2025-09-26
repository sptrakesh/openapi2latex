//
// Created by Rakesh on 25/09/2025.
//

#include "output.hpp"
#include "util/split.hpp"

using std::operator ""s;
using std::operator ""sv;

std::string spt::output::impl::schemaTitle( const spt::model::Schema& schema )
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

void spt::output::impl::writeSchemaForAggregation( const spt::model::Schema& schema, std::string_view title, std::ofstream& file )
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
    const auto cleaned = spt::output::impl::clean( value );
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

  const std::function<void( const spt::model::Schema& prop, const std::string& name )> property = [&file, &cleanedProperty, &boolean, &optdouble, &property]( const spt::model::Schema& prop, const std::string& name )
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
      const auto key = spt::output::impl::referenceKey( prop, "schema"sv );
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

      auto v = std::format( "{:n}", vec );
      boost::algorithm::replace_all( v, "\"", "" );
      v = spt::output::impl::clean( v );
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
