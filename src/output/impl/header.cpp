//
// Created by Rakesh on 25/09/2025.
//

#include "output.hpp"
#include "output/output.hpp"

#include <boost/lexical_cast.hpp>

using std::operator ""sv;

namespace
{
  namespace ph
  {
    void writeExamples( const spt::model::Header& header, std::ofstream& file )
    {
      if ( header.example.has_value() )
      {
        const auto ex = std::any_cast<std::string>( header.example );
        auto line = R"(\hline Example & \verb|)"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
        file.write( ex.data(), static_cast<std::streamsize>( ex.size() ) );
        line = R"(| \\
    )"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      }
      else if ( !header.examples.empty() )
      {
        auto line = R"(\hline Examples &
    \begin{itemize}
    )"sv;
        file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

        for ( const auto& [n, ex] : header.examples )
        {
          if ( !ex.value.has_value() && ex.externalValue.empty() ) continue;
          line = R"(\item \textit{)"sv;
          file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
          file.write( n.data(), static_cast<std::streamsize>( n.size() ) );
          file.write( "}\n", 2 );

          spt::output::impl::writeSummary( ex, file );
          spt::output::impl::writeDescription( ex, file );

          if ( !ex.externalValue.empty() )
          {
            const auto url = boost::lexical_cast<std::string>( ex.externalValue );
            line = R"(See example at \url{)"sv;
            file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
            file.write( url.data(), static_cast<std::streamsize>( url.size() ) );
            file.write( "}\n", 2 );
          }

          if ( ex.value.has_value() )
          {
            line = R"(\verb|)"sv;
            file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
            const auto v = std::any_cast<std::string>( ex.value );
            file.write( "\n", 1 );
            file.write( v.data(), static_cast<std::streamsize>( v.size() ) );
            file.write( "|\n", 2 );
          }
        }
      }
    }
  }
}

void spt::output::impl::writeHeader( std::string_view name, const model::Header& header, std::ofstream& file )
{
  static auto counter = 0;
  auto line = R"(\subsection{)"sv;
  file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
  file.write( name.data(), static_cast<std::streamsize>( name.size() ) );
  file.write( "}\n", 2 );

  const auto starttable = [&file, name]
  {
    const auto idx = std::format( "{}", ++counter );
    auto line = R"(
\begin{center}
\tablefirsthead{}
\tablehead{}
\tabletail{}
\tablecaption{\label{responses:header:)"sv;
    file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
    file.write( name.data(), static_cast<std::streamsize>( name.size() ) );
    file.write( ":", 1 );
    file.write( idx.data(), static_cast<std::streamsize>( idx.size() ) );
    line = R"(}Header details}
\begin{supertabular}{|l|l|}
)"sv;
    file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
  };

  const auto endtable = [&file]
  {
    auto line = R"(\hline
\end{supertabular}
\end{center}
)"sv;
    file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
  };

  const auto boolean = [&file]( std::string_view label, bool value )
  {
    if ( !value ) return;
    auto line = R"(\hline )"sv;
    file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
    file.write( label.data(), static_cast<std::streamsize>( label.size() ) );
    line = R"( & true \\
)"sv;
    file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
  };

  if ( !header.description.empty() )
  {
    const auto desc = convert( header.description );
    file.write( desc.data(), static_cast<std::streamsize>( desc.size() ) );
    file.write( "\n", 1 );
  }

  starttable();

  if ( !header.style.empty() )
  {
    line = R"(\hline \textbf{style} & )"sv;
    file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
    file.write( header.style.data(), static_cast<std::streamsize>( header.style.size() ) );
    line = R"( \\
)"sv;
    file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
  }

  boolean( "required"sv, header.required );
  boolean( "deprecated"sv, header.deprecated );
  boolean( "allowEmptyValue"sv, header.allowEmptyValue );
  boolean( "explode"sv, header.explode );
  ph::writeExamples( header, file );

  endtable();

  if ( header.schema )
  {
    line = R"(\subsubsection{Schema}
\begin{description}
)"sv;
    file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

    writeSchema( header, file );

    line = R"(\end{description}
)"sv;
    file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
  }

  if ( !header.content.empty() )
  {
    line = R"(\subsubsection{Content}
)"sv;
    file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

    for ( const auto& [n, mt] : header.content ) writeMediaType( n, mt, file );
  }

  if ( !header.examples.empty() )
  {
    line = R"(\subsubsection{Examples}
)"sv;
    file.write( line.data(), static_cast<std::streamsize>( line.size() ) );

    for ( const auto& [n, example] : header.examples )
    {
      line = R"(\paragraph{)"sv;
      file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
      file.write( n.data(), static_cast<std::streamsize>( n.size() ) );
      file.write( "\n", 1 );
      writeExample( example, file );
    }
  }
}
