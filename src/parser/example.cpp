//
// Created by Rakesh on 19/09/2025.
//

#include "parser.hpp"
#include "model/example.hpp"

template <>
void spt::parser::parse( model::Example& m, c4::yml::ConstNodeRef node )
{
  static const auto prefix = std::string{ "value: " };
  for ( const auto& child : node.children() )
  {
    if ( child.key() == "summary" ) child >> m.summary;
    if ( child.key() == "description" ) child >> m.description;
    if ( child.key() == "externalValue" ) m.externalValue = parse<boost::urls::url>( child );
    if ( child.key() == "value" )
    {
      if ( child.has_val() ) m.value = std::string{ child.val().begin(), child.val().end() };
      else if ( child.is_seq() || child.is_map() )
      {
        auto v = ryml::emitrs_yaml<std::string>( child );
        m.value = v.starts_with( prefix ) ? v.substr( prefix.size() ) : std::move( v );
      }
    }
  }
}
