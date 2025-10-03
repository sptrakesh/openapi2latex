//
// Created by Rakesh on 25/09/2025.
//

#include <boost/lexical_cast.hpp>


#include "output.hpp"

using std::operator ""sv;

namespace
{
  namespace pmt
  {
    void propertySchema( std::string name, const spt::model::Schema& schema, std::ofstream& file )
    {
      auto line = R"(\item \textbf{)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      auto cleaned = spt::output::impl::clean( std::move( name ) );
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
        const auto title = spt::output::impl::schemaTitle( schema );
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
        spt::output::impl::schemaExamples( schema, file );
      }
    }

    void schemaDetails( std::string name, const spt::model::Schema& schema, std::ofstream& file )
    {
      auto line = R"(\item \textbf{)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      auto cleaned = spt::output::impl::clean( std::move( name ) );
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
        const auto title = spt::output::impl::schemaTitle( schema );
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

        for ( const auto& [ppkey, pp] : schema.properties ) propertySchema( ppkey, pp, file );

        line = R"(\end{itemize}
  )"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }

      line = R"(\end{itemize}
  )"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
    }

    void writeExample( const spt::model::MediaType& mt, std::ofstream& file )
    {
      if ( mt.example.has_value() )
      {
        auto line = R"(\textbf{Example} \verb|)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        const auto v = std::any_cast<std::string>( mt.example );
        file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
        file.write( "|\n", 2 );
      }
      else if ( !mt.examples.empty() )
      {
        for ( const auto& [name, ex] : mt.examples )
        {
          if ( !ex.value.has_value() && ex.externalValue.empty() ) continue;
          auto line = R"(\textbf{Example} - )"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( name.data(), static_cast<std::streamsize>( name.size() ) );

          spt::output::impl::writeSummary( ex, file );
          spt::output::impl::writeDescription( ex, file );

          if ( !ex.externalValue.empty() )
          {
            line = R"(See example at \url{)"sv;
            file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
            const auto v = boost::lexical_cast<std::string>( ex.externalValue );
            file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
            file.write( "}\n", 2 );
          }

          if ( ex.value.has_value() )
          {
            line = R"(
  \verb|)"sv;
            file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

            const auto v = std::any_cast<std::string>( ex.value );
            file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
            file.write( "|\n", 2 );
          }
        }
      }
    }
  }
}

void spt::output::impl::writeMediaType( std::string_view key, const model::MediaType& mt, std::ofstream& file )
{
  auto line = R"(\textbf{)"sv;
  file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
  file.write( key.data(), static_cast<std::streamsize>( key.size() ) );
  file.write( "}\n\n", 3 );

  if ( !mt.schema ) return;

  if ( !mt.schema->summary.empty() )
  {
    const auto sum = convert( mt.schema->summary );
    file.write( sum.data(), static_cast<std::streamsize>( sum.size() ) );
    file.write( "\n\n", 2 );
  }

  writeDescription( *mt.schema, file );

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

  for ( const auto& [pname, prop] : mt.schema->properties ) pmt::schemaDetails( pname, prop, file );

  line = R"(\end{itemize}
)"sv;
  file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

  pmt::writeExample( mt, file );
}
