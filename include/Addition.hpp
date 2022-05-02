// Copyright 2022 VladislavRZ <rzhevskii_vladislav@mail.ru>

#ifndef INCLUDE_ADDITION_HPP_
#define INCLUDE_ADDITION_HPP_

#include <boost/log/core.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/program_options.hpp>

namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace sinks = boost::log::sinks;
namespace po = boost::program_options;

struct CmdArgs {
  std::string log_lvl;
  std::string output;
  std::string input;
  unsigned int threads;
};

bool parse_cmd(int argc, char* argv[], CmdArgs& cmd_args);
bool check_args(CmdArgs& cmd_args);
void log_init();
void set_log_lvl(CmdArgs& cmd);

#endif  // INCLUDE_ADDITION_HPP_
