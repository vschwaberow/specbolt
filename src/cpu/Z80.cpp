#include "cpu/Z80.hpp"
#include "Logger.hpp"

#include <algorithm>

namespace specbolt {
namespace cpu {

Z80::Z80() : memory_(65536) { Reset(); }

void Z80::Reset() {
  pc_ = 0x0000;
  sp_ = 0xFFFF;
  af_ = bc_ = de_ = hl_ = 0x0000;
  std::fill(memory_.Data(), memory_.Data() + memory_.Size(), 0);
}

void Z80::Execute() {
  while (true) {
    uint8_t opcode = Fetch();
    DecodeAndExecute(opcode);
  }
}

uint8_t Z80::Fetch() { return static_cast<uint8_t>(memory_.Data()[pc_++]); }

void Z80::DecodeAndExecute(uint8_t opcode) {
  switch (opcode) {
    case 0x00: // NOP
      NOP();
      break;
    case 0x01: // LD BC,nn
      LD_BC_nn();
      break;
    case 0x03: // INC BC
      INC_BC();
      break;
    case 0x0B: // DEC BC
      DEC_BC();
      break;
    case 0x06: // LD B,n
      LD_B_n();
      break;
    case 0x09: // ADD HL,BC
      ADD_HL_BC();
      break;
    case 0x0A: // LD A,(BC)
      LD_A_BC();
      break;
    case 0x05: // DEC B
      DEC_B();
      break;
    case 0x18: // JR n
      JR_n();
      break;
    // TODO: Add more opcodes here
    default:
      specbolt::Logger::Instance().Log(specbolt::LogLevel::ERROR, "Unknown opcode", "Z80", "DecodeAndExecute");
      break;
  }
}

void Z80::NOP() { specbolt::Logger::Instance().Log(specbolt::LogLevel::DEBUG, "Executed NOP", "Z80", "NOP"); }

void Z80::LD_BC_nn() {
  uint8_t low = static_cast<uint8_t>(memory_.Data()[pc_++]);
  uint8_t high = static_cast<uint8_t>(memory_.Data()[pc_++]);
  bc_ = static_cast<uint16_t>(static_cast<uint16_t>(low) | (static_cast<uint16_t>(high) << 8));
  specbolt::Logger::Instance().Log(specbolt::LogLevel::DEBUG, "Executed LD BC,nn", "Z80", "LD_BC_nn");
}

void Z80::INC_BC() {
  bc_++;
  specbolt::Logger::Instance().Log(specbolt::LogLevel::DEBUG, "Executed INC BC", "Z80", "INC_BC");
}

void Z80::DEC_BC() {
  bc_--;
  specbolt::Logger::Instance().Log(specbolt::LogLevel::DEBUG, "Executed DEC BC", "Z80", "DEC_BC");
}

void Z80::LD_B_n() {
  uint8_t value = static_cast<uint8_t>(memory_.Data()[pc_++]);
  bc_ = (bc_ & 0xFF00) | value;
  specbolt::Logger::Instance().Log(specbolt::LogLevel::DEBUG, "Executed LD B,n", "Z80", "LD_B_n");
}

void Z80::ADD_HL_BC() {
  uint32_t result = hl_ + bc_;
  hl_ = result & 0xFFFF;
  if (result > 0xFFFF) {
    af_ |= 0x10;
  }
  else {
    af_ &= ~0x10;
  }
  specbolt::Logger::Instance().Log(specbolt::LogLevel::DEBUG, "Executed ADD HL,BC", "Z80", "ADD_HL_BC");
}

void Z80::LD_A_BC() {
  uint16_t address = bc_;
  uint8_t value = static_cast<uint8_t>(memory_.Data()[address]);
  af_ = (af_ & 0xFF00) | value;
  specbolt::Logger::Instance().Log(specbolt::LogLevel::DEBUG, "Executed LD A,(BC)", "Z80", "LD_A_BC");
}

void Z80::DEC_B() {
  uint8_t b = static_cast<uint8_t>(bc_ & 0xFF);
  b--;
  bc_ = (bc_ & 0xFF00) | b;
  specbolt::Logger::Instance().Log(specbolt::LogLevel::DEBUG, "Executed DEC B", "Z80", "DEC_B");
}

void Z80::JR_n() {
  int8_t offset = static_cast<int8_t>(Fetch());
  pc_ = static_cast<uint16_t>(static_cast<int16_t>(pc_) + offset);
  specbolt::Logger::Instance().Log(specbolt::LogLevel::DEBUG, "Executed JR n", "Z80", "JR_n");
}

} // namespace cpu
} // namespace specbolt
