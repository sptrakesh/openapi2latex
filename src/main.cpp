//
// Created by Rakesh on 25/12/2021.
//

#include "log/NanoLog.hpp"
#include "output/output.hpp"
#include "parser/parser.hpp"
#include "util/clara.hpp"

#include <iostream>
#include <filesystem>
#include <fstream>

#include "resolver/resolver.hpp"

namespace
{
  namespace run
  {
    void setServers( spt::model::PathItem& pi, const std::vector<spt::model::Server>& servers )
    {
      if ( servers.empty() ) return;
      if ( pi.servers.empty() ) pi.servers = servers;

      if ( pi.servers.empty() ) return;
      if ( pi.get && pi.get->servers.empty() ) pi.get->servers = servers;
      if ( pi.put && pi.put->servers.empty() ) pi.put->servers = servers;
      if ( pi.post && pi.post->servers.empty() ) pi.post->servers = servers;
      if ( pi._delete && pi._delete->servers.empty() ) pi._delete->servers = servers;
      if ( pi.options && pi.options->servers.empty() ) pi.options->servers = servers;
      if ( pi.head && pi.head->servers.empty() ) pi.head->servers = servers;
      if ( pi.patch && pi.patch->servers.empty() ) pi.patch->servers = servers;
      if ( pi.trace && pi.trace->servers.empty() ) pi.trace->servers = servers;
    }

    void setSecurity( spt::model::PathItem& pi, const std::vector<spt::model::SecurityRequirement>& security )
    {
      if ( security.empty() ) return;

      if ( pi.get && pi.get->security.empty() ) pi.get->security = security;
      else LOG_DEBUG << "Path item " << pi.get->operationId << " has custom security requirement.";

      if ( pi.put && pi.put->security.empty() ) pi.put->security = security;
      else LOG_DEBUG << "Path item " << pi.put->operationId << " has custom security requirement.";

      if ( pi.post && pi.post->security.empty() ) pi.post->security = security;
      else LOG_DEBUG << "Path item " << pi.post->operationId << " has custom security requirement.";

      if ( pi._delete && pi._delete->security.empty() ) pi._delete->security = security;
      else LOG_DEBUG << "Path item " << pi._delete->operationId << " has custom security requirement.";

      if ( pi.options && pi.options->security.empty() ) pi.options->security = security;
      else LOG_DEBUG << "Path item " << pi.options->operationId << " has custom security requirement.";

      if ( pi.head && pi.head->security.empty() ) pi.head->security = security;
      else LOG_DEBUG << "Path item " << pi.head->operationId << " has custom security requirement.";

      if ( pi.patch && pi.patch->security.empty() ) pi.patch->security = security;
      else LOG_DEBUG << "Path item " << pi.patch->operationId << " has custom security requirement.";

      if ( pi.trace && pi.trace->security.empty() ) pi.trace->security = security;
      else LOG_DEBUG << "Path item " << pi.trace->operationId << " has custom security requirement.";
    }

    int process( const spt::model::Configuration& conf )
    {
      LOG_INFO << "Parsing OpenAPI specifications from " << conf.input;
      auto path = std::filesystem::path( conf.input );
      if ( !std::filesystem::exists( conf.input ) )
      {
        LOG_CRIT << "Input file " << conf.input << " does not exist.";
        return 1;
      }

      if ( !std::filesystem::is_regular_file( conf.input ) )
      {
        LOG_CRIT << "Input file " << conf.input << " is not a regular file.";
        return 2;
      }

      std::ifstream f( conf.input, std::ios::in | std::ios::binary );
      const auto size = std::filesystem::file_size( path );
      std::string contents( size, '\0' );
      f.read( contents.data(), static_cast<std::streamsize>( size ) );

      const auto tree = ryml::parse_in_place( contents.data() );
      const auto root = tree.rootref();
      if ( !root.is_map() )
      {
        LOG_CRIT << "Invalid input specification file " << conf.input;
        return 3;
      }

      auto openapi = spt::model::OpenAPI{};
      openapi._referenceURI = std::filesystem::weakly_canonical( path ).make_preferred().string();
      spt::parser::parse( openapi, root );

      spt::resolver::resolve( openapi, openapi._referenceURI );

      for ( auto& [_, pi] : openapi.paths ) setServers( pi, openapi.servers );
      for ( auto& [_, pi] : openapi.paths ) setSecurity( pi, openapi.security );

      const auto outfile = spt::output::generate( openapi, conf );
      LOG_INFO << "Generated output file " << outfile;
      LOG_INFO << "Run `xelatex` multiple times until references are resolved." << outfile;

      return 0;
    }
  }
}

int main( int argc, char const * const * argv )
{
  using clara::Opt;
  spt::model::Configuration config;
  std::string logLevel{"info"};
#if defined(__unix__) && !defined(__APPLE__)
  std::string dir{"/opt/spt/logs/"};
#else
  std::string dir{"/tmp/"};
#endif
  bool help = false;
  bool console = false;

  auto options = clara::Help(help) |
      Opt(config.input, "openapi.yaml")["-i"]["--input"]("The input OpenAPI YAML file.") |
      Opt(config.output, "/tmp")["-o"]["--output"]("The fully qualified path for the output LaTeX files.") |
      Opt(config.author, "OpenAPI2LaTeX Generator")["-a"]["--author"]("The author of the document.") |
      Opt(config.footer, "Proprietary and Confidential")["-f"]["--footer"]("The right footer text for the document.") |
      Opt(config.font, "Helvetica Neue")["-t"]["--font"]("The font to use for the document (default Helvetica Neue).") |
      Opt(config.operationSummary)["-s"]["--operation-summary"]("Use operation summary as title instead of operationId.") |
      Opt(config.cmark)["-m"]["--use-cmark"]("Use cmark to convert info.description to latex.") |
      Opt(console)["-c"]["--console"]("Log to console (default off)") |
      Opt(logLevel, "info")["-l"]["--log-level"]("Log level to use [debug|info|warn|critical] (default info).") |
      Opt(dir, "/tmp/")["-z"]["--log-dir"]("Log directory (default /tmp/)");

  if ( auto result = options.parse( clara::Args( argc, argv ) ); !result )
  {
    std::cerr << "Error in command line: " << result.errorMessage() << std::endl;
    exit( 1 );
  }

  if ( help )
  {
    options.writeToStream( std::cout );
    exit( 0 );
  }

  if ( config.input.empty() || config.output.empty() )
  {
    options.writeToStream( std::cout );
    exit( 1 );
  }

  if ( logLevel == "debug" ) nanolog::set_log_level( nanolog::LogLevel::DEBUG );
  else if ( logLevel == "info" ) nanolog::set_log_level( nanolog::LogLevel::INFO );
  else if ( logLevel == "warn" ) nanolog::set_log_level( nanolog::LogLevel::WARN );
  else if ( logLevel == "critical" ) nanolog::set_log_level( nanolog::LogLevel::CRIT );
  nanolog::initialize( nanolog::GuaranteedLogger(), dir, "openapi2latex", console );

  return run::process( config );
}
