#include "cpu/Z80.hpp"

#include <algorithm>

namespace specbolt {
namespace cpu {

Z80::Z80() : memory_(65536) {
  Reset();
}

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

uint8_t Z80::Fetch() {
  return memory_.Data()[pc_++];
}

void Z80::DecodeAndExecute(uint8_t opcode) {
  switch (opcode) {
    case 0x00:  // NOP
      break;
    case 0x01:  // LD BC,nn
      bc_ = memory_.Data()[pc_++] | (memory_.Data()[pc_++] << 8);
      break;
    // TODO: Add more opcodes here
    default:
      // TODO: Handle unknown opcode
      break;
  }
}

}
}