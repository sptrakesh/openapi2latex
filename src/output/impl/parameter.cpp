//
// Created by Rakesh on 26/09/2025.
//

#include <boost/lexical_cast.hpp>


#include "collectors.hpp"
#include "output.hpp"

using std::operator ""sv;

namespace
{
  namespace pparam
  {
    void writeExamples( const spt::model::Parameter& param, std::ofstream& file )
    {
      if ( param.example.has_value() )
      {
        auto line = R"(\item \textit{example} - \verb|)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        const auto v = std::any_cast<std::string>( param.example );
        file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
        file.write( "|\n", 2 );
        return;
      }

      if ( param.examples.empty() ) return;

        auto line = R"(\item \textit{examples}
\begin{description}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

      for ( const auto& [en, ex] : param.examples )
      {
        if ( !ex.value.has_value() && !ex.externalValue.empty() ) continue;

        line = R"(\item \textit{)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        const auto cleaned = spt::output::impl::clean( en );
        file.write( cleaned.data(), static_cast<std::streamsize>( cleaned.size() ) );
        line = R"(} - )"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

        if ( !ex.summary.empty() )
        {
          const auto s = spt::output::impl::clean( ex.summary );
          file.write( s.data(), static_cast<std::streamsize>( s.size() ) );
          file.write( "\n", 1 );
        }

        if ( !ex.description.empty() )
        {
          const auto s = spt::output::impl::clean( ex.description );
          file.write( s.data(), static_cast<std::streamsize>( s.size() ) );
          file.write( "\n", 1 );
        }

        if ( ex.value.has_value() )
        {
          line = R"(\verb|)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          const auto v = std::any_cast<std::string>( ex.value );
          file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
          file.write( "|\n", 2 );
        }

        if ( !ex.externalValue.empty() )
        {
          line = R"(See example at \url{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          const auto v = boost::lexical_cast<std::string>( ex.externalValue );
          file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
          file.write( "}\n", 2 );
        }
      }

      line = R"(\end{description}
)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
    }
  }
}

void spt::output::impl::writeParameter( const model::Parameter& param, std::ofstream& file, bool initial )
{
  if ( !initial )
  {
    auto line = R"(\item \textbf{)"sv;
    file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
    const auto name = clean( param.name );
    file.write( name.data(), static_cast<std::streamsize>( name.size() ) );
    file.write( "} ", 2 );
  }

  writeDescription( param, file );

  if ( !initial )
  {
    const auto key = referenceKey( param, "parameter"sv );
    if ( Collectors::instance().parameterMap.contains( key ) )
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

  pparam::writeExamples( param, file );

  line = R"(\end{description}
)"sv;
  file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
}
