// Copyright 2022 VladislavRZ <rzhevskii_vladislav@mail.ru>
#include "Addition.hpp"

bool parse_cmd(int argc, char* argv[], CmdArgs& cmd_args) {
  // Add options
  po::options_description visible("Options");
  visible.add_options()("help, h", "produce help message")(
      "log-level",
      po::value<std::string>(&cmd_args.log_lvl)->default_value("error"),
      "info|warning|error")(
      "thread-count",
      po::value<unsigned int>(&cmd_args.threads)->default_value(3),
      "count of threads")(
      "output",
      po::value<std::string>(&cmd_args.output)->default_value("OutputDB"),
      "path/to/output");

  po::options_description hidden("Hidden options");
  hidden.add_options()("input-file",
                       po::value<std::string>(&cmd_args.input),
                       "input file");

  // Add positional options
  po::positional_options_description p;
  p.add("input-file", -1);

  // Group options
  po::options_description cmd_options;
  cmd_options.add(visible).add(hidden);

  // Parse options
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv)
                .options(cmd_options)
                .positional(p)
                .run(),
            vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << "\nUsage:\n"
              << argv[0] << " [options] <path/to/input>\n"
              << std::endl;
    std::cout << visible << std::endl;
    return false;
  }

  return true;
}

bool check_args(CmdArgs& cmd_args) {
  if (cmd_args.input.empty()) {
    BOOST_LOG_TRIVIAL(warning)
        << "Warning: Program can't work without input file.";
    return false;
  }

  if (cmd_args.threads == 0) {
    BOOST_LOG_TRIVIAL(warning) << "Warning: Threads count can't be zero.";
    return false;
  }

  if (cmd_args.log_lvl != "info" &&
      cmd_args.log_lvl != "warning" &&
      cmd_args.log_lvl != "error") {
    BOOST_LOG_TRIVIAL(warning) << "Unknown log type."
                               << " Log-level have set to info.";
  }

  return true;
}

void log_init() {
  logging::add_common_attributes();
  logging::add_file_log(
      keywords::file_name = "info_%N.log",
      keywords::target_file_name = "info_%N.log",
      keywords::rotation_size = 5 * 1024 * 1024,
      keywords::time_based_rotation =
          sinks::file::rotation_at_time_point(0, 0, 0),
      keywords::filter = logging::trivial::severity <= logging::trivial::info,
      keywords::format = "[%ThreadID%] %Message%");

  logging::add_common_attributes();
  logging::add_file_log(keywords::file_name = "warning_%N.log",
                        keywords::target_file_name = "warning_%N.log",
                        keywords::rotation_size = 5 * 1024 * 1024,
                        keywords::time_based_rotation =
                            sinks::file::rotation_at_time_point(0, 0, 0),
                        keywords::filter = logging::trivial::severity ==
                                           logging::trivial::warning,
                        keywords::format = "[%ThreadID%] %Message%");

  logging::add_common_attributes();
  logging::add_file_log(
      keywords::file_name = "error_%N.log",
      keywords::target_file_name = "error_%N.log",
      keywords::rotation_size = 5 * 1024 * 1024,
      keywords::time_based_rotation =
          sinks::file::rotation_at_time_point(0, 0, 0),
      keywords::filter = logging::trivial::severity >= logging::trivial::error,
      keywords::format = "[%ThreadID%] %Message%");

  logging::add_console_log(std::cout,
                           keywords::format = "[%Severity%] %Message%");
}

void set_log_lvl(CmdArgs& cmd) {
  if (cmd.log_lvl == "warning") {
    logging::core::get()->set_filter(logging::trivial::severity >=
                                     logging::trivial::warning);
  } else if (cmd.log_lvl == "error") {
    logging::core::get()->set_filter(logging::trivial::severity >=
                                     logging::trivial::error);
  } else {
    logging::core::get()->set_filter(logging::trivial::severity >=
                                     logging::trivial::info);
  }
}
