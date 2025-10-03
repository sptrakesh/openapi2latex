//
// Created by Rakesh on 25/09/2025.
//

#include "output.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>

using std::operator ""sv;

void spt::output::impl::writeInput( const std::filesystem::path& path, std::ofstream& file )
{
  auto line = R"(\input{)"sv;
  file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
  auto input = path.string();
  file.write( input.data(), static_cast<std::streamsize>( input.size() ) );
  line = R"(}
)"sv;
  file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
}

std::string spt::output::impl::clean( std::string text )
{
  boost::replace_all( text, "&", R"(\&)" );
  boost::replace_all( text, R"($)", R"(\$)" );
  boost::replace_all( text, R"([)", R"(\[)" );
  boost::replace_all( text, R"(])", R"(\])" );
  boost::replace_all( text, R"({)", R"(\})" );
  boost::replace_all( text, R"(_)", R"(\textunderscore )" );
  boost::replace_all( text, R"(#)", R"(\#)" );
  boost::replace_all( text, R"(%)", R"(\%)" );
  return text;
}

void spt::output::impl::writeExample( const model::Example& example, std::ofstream& file )
{
  writeSummary( example, file );
  writeDescription( example, file );

  if ( !example.externalValue.empty() )
  {
    const auto url = boost::lexical_cast<std::string>( example.externalValue );
    auto line = R"(See example at \url{)"sv;
    file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
    file.write( url.data(), static_cast<std::streamsize>( url.size() ) );
    file.write( "}\n", 2 );
  }

  if ( example.value.has_value() )
  {
    const auto value = std::any_cast<std::string>( example.value );
    auto line = R"(\verb|)"sv;
    file.write( line.data(), static_cast<std::streamsize>( line.size() ) );
    file.write( value.data(), static_cast<std::streamsize>( value.size() ) );
    file.write( "|\n", 2 );
  }
}
