#include "headers/emulator.h"


// Simple helper for file as single line. Feel free to change it or delete it completely
std::optional<std::string> readStringFromFile(const std::string& filename) {
  std::ifstream file{filename};

  if (!file) return {};

  std::stringstream buf;
  buf << file.rdbuf();

  return buf.str();
}

int main() {
  // For writing test you can write programs directly in raw string literals
  std::string factorial = R"(
    Mov R0 5
    Mov R1 1    
    Jmpz 6  
                  
    Mul R1 R0   
    Sub R0 1    
    Jmp 2       

    Mov R0 R1
  )";

  // The result should be 120
  int fact5 = Emulator::emulate(factorial);
  
  // Or you can use file IO
  std::string filename = "factorial.vrisc";
  std::optional<std::string> program = readStringFromFile("factorial.vrisc");

  if (!program) {
    std::cerr << "Can't open file" << std::endl;
    exit(1);
    // return 1;
  }

  // And this also should be 120
  int another_fact = Emulator::emulate(*program);

  // TODO: remeber the tests is very important!
  return 0;
}
