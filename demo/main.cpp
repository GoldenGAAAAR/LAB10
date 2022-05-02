#include <boost/program_options.hpp>

#include "ChecksumCalc.hpp"

namespace  po = boost::program_options;

bool parse_cmd(int argc, char* argv[], CmdArgs& cmd_args) {
  // Add options
  po::options_description visible("Options");
  visible.add_options()
      ("help, h", "produce help message")
      ("log-level", po::value<std::string>(&cmd_args.log_lvl)->default_value("error"),
       "info|warning|error")
      ("thread-count",
       po::value<unsigned int>(&cmd_args.threads)->default_value(3),
       "count of threads")
      ("output",
       po::value<std::string>(&cmd_args.output)->default_value("OutputDataBase"),
        "path/to/output")
      ;

  po::options_description hidden("Hidden options");
  hidden.add_options()
      ("input-file", po::value<std::string>(&cmd_args.input), "input file");

  // Add positional options
  po::positional_options_description p;
  p.add("input-file", -1);

  // Parse options
  po::options_description cmd_options;
  cmd_options.add(visible).add(hidden);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(cmd_options).positional(p).run(), vm);
  po::notify(vm);

  // Check errors
  if (vm.count("help")) {
    std::cout << "\nUsage:\n" << argv[0] << " [options] <path/to/input>\n" << std::endl;
    std::cout << visible << std::endl;
    return false;
  }

  if (!vm.count("input-file"))
  {
    std::cout << "Program can't work without input file." << std::endl;
    return false;
  }

  if (cmd_args.threads == 0) {
    std::cout << "Threads count can't be zero." << std::endl;
    return false;
  }

  return true;
}

// TODO: Сделать логи
// TODO: Проверить входные аргументы, уровень логгирования
// TODO: Сделать адекватную функцию вывода БД
// TODO: Многопоточная запись

int main(int argc, char* argv[]) {
  try {
    CmdArgs cmd_args;
    if (!parse_cmd(argc, argv, cmd_args))
      return 0;

    ChecksumCalc calc;
    calc.run(cmd_args);
  }
  catch(std::exception &e)
  {
    std::cout << "Exception: " << e.what() << std::endl;
    return 1;
  }
  catch(...)
  {
    std::cout << "Unknown error!" << std::endl;
  }

  return 0;
}