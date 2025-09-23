//
// Created by Rakesh on 22/09/2025.
//

#include <catch2/catch_test_macros.hpp>
#include "../src/output/output.hpp"

using std::operator ""s;
using std::operator ""sv;

SCENARIO( "Markdown to LaTeX conversion", "convert" )
{
  using namespace spt::output;

  GIVEN( "Bullet list conversion support" )
  {
    WHEN( "List begins with *" )
    {
      auto md = R"(* Item 1
* Item 2
* Item 3
)"sv;

      auto expected = R"(\begin{itemize}
\item Item 1
\item Item 2
\item Item 3
\end{itemize}
)"s;

      auto result = convert( md );
      CHECK( result == expected );
      CHECK( result.contains( R"(\begin{itemize})" ) );
      CHECK( result.contains( R"(\item Item 1)" ) );
      CHECK( result.contains( R"(\item Item 2)" ) );
      CHECK( result.contains( R"(\item Item 3)" ) );
      CHECK( result.contains( R"(\end{itemize})" ) );
    }

    AND_WHEN( "List begins with -" )
    {
      auto md = R"(- Item 1
- Item 2
- Item 3
)"sv;

      auto expected = R"(\begin{itemize}
\item Item 1
\item Item 2
\item Item 3
\end{itemize}
)"s;

      auto result = convert( md );
      CHECK( result == expected );
    }

    AND_WHEN( "List with 2 levels" )
    {
      auto md = R"(* Item 1
* Item 2
  * Item 2 a
  * Item 2 b
* Item 3
)"sv;

      auto expected = R"(\begin{itemize}
\item Item 1
\item Item 2
\begin{itemize}
\item Item 2 a
\item Item 2 b
\end{itemize}
\item Item 3
\end{itemize}
)"s;

      auto result = convert( md );
      CHECK( result == expected );
    }

    AND_WHEN( "List with 3 levels" )
    {
      auto md = R"(* Item 1
* Item 2
  * Item 2 a
  * Item 2 b
    * Item 2 b i
    * Item 2 b ii
)"sv;

      auto expected = R"(\begin{itemize}
\item Item 1
\item Item 2
\begin{itemize}
\item Item 2 a
\item Item 2 b
\begin{itemize}
\item Item 2 b i
\item Item 2 b ii
\end{itemize}
\end{itemize}
\end{itemize}
)"s;

      auto result = convert( md );
      CHECK( result == expected );
    }
  }

  GIVEN( "Link conversion support" )
  {
    WHEN( "Text with a link" )
    {
      auto md = R"(Some line with [link](https://sptci.com/) embedded)"sv;
      auto expected = R"(Some line with \href{https://sptci.com/}{link} embedded)"s;

      auto result = convert( md );
      CHECK( result == expected );
    }

    AND_WHEN( "Text with multiple links" )
    {
      auto md = R"(Some text with [link](https://sptci.com/) embedded and [profile](https://sptrakesh.github.io/introduction.html).)"sv;
      auto expected = R"(Some text with \href{https://sptci.com/}{link} embedded and \href{https://sptrakesh.github.io/introduction.html}{profile}.)"s;

      auto result = convert( md );
      CHECK( result == expected );
    }
  }

  GIVEN( "Style conversion support" )
  {
    WHEN( "Bold with asterisks" )
    {
      auto md = R"(Micro-services for **ReelSense** data interactions.)"sv;
      auto expected = R"(Micro-services for \textbf{ReelSense} data interactions.)";
      auto result = convert( md );
      CHECK( result == expected );
    }

    AND_WHEN( "Bold with underscore" )
    {
      auto md = R"(Micro-services for __ReelSense__ data interactions.)"sv;
      auto expected = R"(Micro-services for \textbf{ReelSense} data interactions.)";
      auto result = convert( md );
      CHECK( result == expected );
    }

    AND_WHEN( "Italicised text with asterisks" )
    {
      auto md = R"(*Any* logged in user may invoke this endpoint.)"sv;
      auto expected = R"(\textit{Any} logged in user may invoke this endpoint.)";
      auto result = convert( md );
      CHECK( result == expected );
    }

    AND_WHEN( "Italicised text with underscore" )
    {
      auto md = R"(_Any_ logged in user may invoke this endpoint.)"sv;
      auto expected = R"(\textit{Any} logged in user may invoke this endpoint.)";
      auto result = convert( md );
      CHECK( result == expected );
    }

    AND_WHEN( "Underscore within in-line code block" )
    {
      auto md = R"(Since this is a full replace, the replacement document must be the full
document (the `_id` field is optional).)"sv;
      auto expected = R"(Since this is a full replace, the replacement document must be the full
document (the \texttt{\textunderscore id} field is optional).)";
      auto result = convert( md );
      CHECK( result == expected );
    }

    AND_WHEN( "Complex multi-paragraph test" )
    {
      auto md = R"(Structure for a general purpose replace request.  Replace is expressed
as a combination of an update `filter` query (should return a single
matching document), and the `replace` document to replace the existing
document in the specified `database:collection`.

Since this is a full replace, the replacement document must be the full
document (the `_id` field is optional).

The post-update document is retrieved (if `_id` is not included) to create
the version history document.)"sv;
      auto expected = R"(Structure for a general purpose replace request.  Replace is expressed
as a combination of an update \texttt{filter} query (should return a single
matching document), and the \texttt{replace} document to replace the existing
document in the specified \texttt{database:collection}.

Since this is a full replace, the replacement document must be the full
document (the \texttt{\textunderscore id} field is optional).

The post-update document is retrieved (if \texttt{\textunderscore id} is not included) to create
the version history document.)";
      auto result = convert( md );
      CHECK( result == expected );
    }
  }
}
