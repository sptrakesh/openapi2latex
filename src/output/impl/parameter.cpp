//
// Created by Rakesh on 26/09/2025.
//

#include "collectors.hpp"
#include "output.hpp"

using std::operator ""sv;

void spt::output::impl::writeParameter( const model::Parameter& param, std::ofstream& file, bool initial )
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

  if ( param.schema ) spt::output::impl::writeSchema( param, file );

  line = R"(\end{description}
)"sv;
  file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
}
