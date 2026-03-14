#include "../headers/json.hpp"
#include <fstream>
#include <iostream>
#include <string>

using json = nlohmann::json;
int main(int argc, char *argv[]) {
  json j;
  std::ofstream FILE("../DATA.JSON");
  if (!FILE.is_open()) {
    std::cerr << "COF error. (Coulnt Open File)" << '\n';
  }

  if (argc < 2) {
    std::cout
        << "Welcome to chezOS cli! \n This cli is used for the ChezOS "
           "terminal, but you can also run it from here, the windows terminal!"
        << std::endl;
    return 0;
  }
  // Ring access, like giving user access to ring 2, 1, 0.
  if (std::string(argv[1]) == "ring" && !argv[2]) {
    std::cout
        << "This command, gives access to rings. This command is one-time, so "
           "you need chezOS to be on. Each ring means higher access for the "
           "user.\n "
           "For example, ring 3 is the user. Yes, he can end processes and "
           "delete the whole DISK folder. RING 2 is what services like this "
           "ChezOSCLI can do or AI.dll do.\n"
           "Ring 1 is where the fun begins. You can execute code with arg[1] "
           "that has string 'exec', that acts similarly to sudo but absolutely "
           "no hesistation from the OS."
           "Ring 0 is.... FUN. Theres nothing really interesting here but you "
           "can execute integrated SMG code, yk that language i made right? ye "
           "i am gonna use it. You can run commands like stop, wait, idk. you "
           "can only do it by running arg[1] command with 'smg'. "
           "\n\n\n\n\n\n\n";
  } else if (std::string(argv[1]) == "ring" && std::string(argv[2]) == "2") {
    // Write to file
    j["ring"] = 2;
    FILE << j.dump(4);
    std::cout
        << "cmd finished, added access to ring 2 by writing to DATA.JSON\n";
  } else if (std::string(argv[1]) == "ring" && std::string(argv[2]) == "1") {
    j["ring"] = 1;
    FILE << j.dump(4);
    std::cout
        << "cmd finished, added access to ring 1 by writing to DATA.JSON\n";

  } else if (std::string(argv[1]) == "ring" && std::string(argv[2]) == "0") {
    j["ring"] = 0;
    FILE << j.dump(4);
    std::cout
        << "cmd finished, added access to ring 0 by writing to DATA.JSON\n";
  }

  // exec

  if (std::string(argv[1]) == "exec") {
  }
}