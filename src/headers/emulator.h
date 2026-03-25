#ifndef __EMULATOR_H__
#define __EMULATOR_H__

#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <optional>
#include <regex>
#include <unordered_map>
#include <functional>
#include <algorithm>


namespace Emulator {
  enum Reg {
    R0, R1, R2, R3
  };

//   class EmulatorState;

  struct EmulatorState {
    static const size_t regs_size = 4;
    static const size_t mem_size  = 1024;

    std::vector<int> _registers{regs_size}; 
    std::vector<int> _mem{mem_size};

    size_t _pc = 0; // program counter register
    EmulatorState(): _registers(regs_size, 0), _mem(mem_size, 0) {};
  };

  // TODO: implement all instructions listed in ISA. This class should be base class for all insturctions
  class Instruction {
  public:
    virtual void eval(EmulatorState& emul) = 0;
    virtual ~Instruction() {};
  };

  class UnaryReg : public Instruction {
    protected:
        Reg dst;
    public:
        UnaryReg(Reg r): dst(r) {};
        ~UnaryReg() = default;
  };

  class UnaryX : public Instruction {
    protected:
        int val;
    public:
        UnaryX(int x): val(x) {};
        ~UnaryX() = default;
  };

  class BinaryRegReg : public Instruction {
    protected:
        Reg dst;
        Reg val;
    public:
        BinaryRegReg(Reg r, Reg x): dst(r), val(x) {};
        ~BinaryRegReg() override = default;
  };

  class BinaryRegX : public Instruction {
    protected:
        Reg dst;
        int val;
    public:
        BinaryRegX(Reg r, int x): dst(r), val(x) {};
        ~BinaryRegX() = default;
  };

  class MovReg : public BinaryRegReg {
  public:
    using BinaryRegReg::BinaryRegReg;
    void eval(EmulatorState& emul) override {
        emul._registers[dst] = emul._registers[val];
    }
  };

  class MovX : public BinaryRegX {
  public:
    using BinaryRegX::BinaryRegX;
    void eval(EmulatorState& emul) override {
        emul._registers[dst] = val;
    }
  };

  class AddRegReg : public BinaryRegReg {
  public:
    using BinaryRegReg::BinaryRegReg;
    void eval(EmulatorState& emul) override {
        emul._registers[dst] += emul._registers[val];
    }
  };

  class AddRegX : public BinaryRegX {
  public:
    using BinaryRegX::BinaryRegX;
    void eval(EmulatorState& emul) override {
        emul._registers[dst] += val;
    }
  };

  class SubRegReg : public BinaryRegReg {
  public:
    using BinaryRegReg::BinaryRegReg;
    void eval(EmulatorState& emul) override {
        emul._registers[dst] -= emul._registers[val];
    }
  };

  class SubRegX : public BinaryRegX {
  public:
    using BinaryRegX::BinaryRegX;
    void eval(EmulatorState& emul) override {
        emul._registers[dst] -= val;
    }
  };

  class MulRegReg : public BinaryRegReg {
  public:
    using BinaryRegReg::BinaryRegReg;
    void eval(EmulatorState& emul) override {
        emul._registers[dst] *= emul._registers[val];
    }
  };

  class MulRegX : public BinaryRegX {
  public:
    using BinaryRegX::BinaryRegX;
    void eval(EmulatorState& emul) override {
        emul._registers[dst] *= val;
    }
  };
  
  class DivRegReg : public BinaryRegReg {
  public:
    using BinaryRegReg::BinaryRegReg;
    void eval(EmulatorState& emul) override {
        emul._registers[dst] /= emul._registers[val];
    }
  };

  class DivRegX : public BinaryRegX {
  public:
    using BinaryRegX::BinaryRegX;
    void eval(EmulatorState& emul) override {
        emul._registers[dst] /= val;
    }
  };

  class Load : public BinaryRegX {
  public:
    using BinaryRegX::BinaryRegX;
    void eval(EmulatorState& emul) override {
        emul._registers[dst] = emul._mem[val];
    }
  };

  class Store : public BinaryRegX {
  public:
    using BinaryRegX::BinaryRegX;
    void eval(EmulatorState& emul) override {
        emul._mem[val] = emul._registers[dst];
    }
  };
  
  class JmpX : public UnaryX {
  public:
    using UnaryX::UnaryX;
    void eval(EmulatorState& emul) override {
        emul._pc = val - 1;
    }
  };
  
  class JmpzX : public JmpX {
  public:
    using JmpX::JmpX;
    void eval(EmulatorState& emul) override {
        if (!(emul._registers[R0])) { JmpX::eval(emul); }
    }
  };
  
  std::vector<std::string> splitSpase(const std::string& st) {
    std::string token;
    std::stringstream ss(st);
    std::vector<std::string> result;

    while (ss >> token) {
        result.push_back(token);
    }
    return result;
}

   bool isRegister(const std::string& s) {
    return (s.length() == 2 && s[0] == 'R' && s[1] >= '0' && s[1] <= '3');
  }

  Reg parse_reg(const std::string& s) {
    if (s == "R0") return R0;
    if (s == "R1") return R1;
    if (s == "R2") return R2;
    if (s == "R3") return R3;
    throw std::invalid_argument("Unknown Register: " + s);
  }
  
  using InstrFactory = std::function<Instruction*(const std::vector<std::string>&)>;
  
  /* This function accepts the program written in the vrisc assembly
   * If the input program is correct, returns sequence of insturction, corresponding to the input program.
   * If the input text is incorrect, the behaviour is undefined
   */
  std::vector<Instruction*> parse(const std::string& program) {
    static const std::unordered_map<std::string, InstrFactory> factoriesUnary = {
        {"jmp", [](const std::vector<std::string>& args) -> Instruction* {
            return new JmpX(std::stoi(args[1]));
        }},
        {"jmpz", [](const std::vector<std::string>& args) -> Instruction* {
            return new JmpzX(std::stoi(args[1]));
        }},
    };

    static const std::unordered_map<std::string, InstrFactory> factoriesBinary = {
        {"mov", [](const std::vector<std::string>& args) -> Instruction* {
            Reg dst = parse_reg(args[1]);
            if (isRegister(args[2])) {
                return new MovReg(dst, parse_reg(args[2]));
            }
            return new MovX(dst, std::stoi(args[2]));
        }},
        {"add", [](const std::vector<std::string>& args) -> Instruction* {
            Reg dst = parse_reg(args[1]);
            if (isRegister(args[2])) {
                return new AddRegReg(dst, parse_reg(args[2]));
            }
            return new AddRegX(dst, std::stoi(args[2]));
        }},
        {"sub", [](const std::vector<std::string>& args) -> Instruction* {
            Reg dst = parse_reg(args[1]);
            if (isRegister(args[2])) {
                return new SubRegReg(dst, parse_reg(args[2]));
            }
            return new SubRegX(dst, std::stoi(args[2]));
        }},
        {"mul", [](const std::vector<std::string>& args) -> Instruction* {
            Reg dst = parse_reg(args[1]);
            if (isRegister(args[2])) {
                return new MulRegReg(dst, parse_reg(args[2]));
            }
            return new MulRegX(dst, std::stoi(args[2]));
        }},
        {"div", [](const std::vector<std::string>& args) -> Instruction* {
            Reg dst = parse_reg(args[1]);
            if (isRegister(args[2])) {
                return new DivRegReg(dst, parse_reg(args[2]));
            }
            return new DivRegX(dst, std::stoi(args[2]));
        }},
        {"load", [](const std::vector<std::string>& args) -> Instruction* {
            return new Load(parse_reg(args[1]), std::stoi(args[2]));
        }},
        {"store", [](const std::vector<std::string>& args) -> Instruction* {
            return new Store(parse_reg(args[1]), std::stoi(args[2]));
        }},
    };

    std::string line;
    std::vector<std::string> arr;
    std::stringstream ss(program);
    std::vector<Instruction*> result;
    // size_t i = 0;
    while (std::getline(ss, line)) {
        // std::cout << i << "): \"" << line << "\"\n-------------\n";
        arr = splitSpase(line);
        if (arr.empty()) { continue; }
        // std::cout << i << "): \"" << line << "\"\n-------------\n";

        std::string& mnemonic = arr[0];
        std::transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(), ::tolower);
        if (arr.size() != 2 && arr.size() != 3) { throw std::invalid_argument("Invalid arguments for: " + mnemonic + '\n'); }

        auto factories = arr.size() == 3 ? factoriesBinary : factoriesUnary;
        auto factory = factories.find(mnemonic);

        if (factory == factories.end()) { throw std::invalid_argument("Unknown instruction: " + mnemonic); } 
        result.push_back(factory->second(arr));
        // i++;
    }
    return result;
  }


  /* Emulate receive a program, written in the vrisc assembly,
   * in case of the correct program, emulate returns R0 value at the end of the emulation.
   * If the program is incorrect, that is, either its text is not vrisc assembly language or it contains UB(endless cycles),
   * the behaviour of emulate if also undefined. Handle these cases in any way.
   */
  int emulate(const std::string& program_text) {
    std::vector<Instruction*> program = parse(program_text);

    EmulatorState state;

    while (state._pc < program.size()) {
      program[state._pc]->eval(state);
      state._pc++;
    }
R"(
        Mov R0 10        
        Mov R1 0         
    
        Jmpz 5           
        Add R1 R0        
        Sub R0 1         
        Jmp 2            
        Mov R0 R1        
    )"
    for (size_t i = 0; i < program.size(); i++) {
      delete program[i];
    }

    return state._registers[R0];
  }

}


#endif // __EMULATOR_H__