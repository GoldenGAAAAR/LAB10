#include "ChecksumCalc.hpp"

int main(int argc, char* argv[]) {
  try {
    CmdArgs cmd_args;

    // Parse command line arguments
    log_init();
    if (!parse_cmd(argc, argv, cmd_args)) return 0;
    set_log_lvl(cmd_args);
    if (!check_args(cmd_args)) return 0;

    // Start
    ChecksumCalc calc;
    calc.run(cmd_args);
  } catch (std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << "Exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    BOOST_LOG_TRIVIAL(error) << "Exception: Unknown error!" << std::endl;
    return 1;
  }

  return 0;
}