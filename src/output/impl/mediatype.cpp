//
// Created by Rakesh on 25/09/2025.
//

#include "output.hpp"

using std::operator ""sv;

void spt::output::impl::writeMediaType( std::string_view key, const model::MediaType& mt, std::ofstream& file )
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
    const auto skey = spt::output::impl::referenceKey( *mt.schema, "schema"sv );
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
      const auto skey = spt::output::impl::referenceKey( schema, "schema"sv );
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
      const auto skey = spt::output::impl::referenceKey( schema, "schema"sv );
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
