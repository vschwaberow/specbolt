#pragma once

#include <cstdint>
#include "ShmId.hpp"

namespace specbolt {
namespace cpu {

class Z80 {
 public:
  Z80();
  void Reset();
  void Execute();

 private:
  uint16_t pc_;
  uint16_t sp_;
  uint16_t af_, bc_, de_, hl_;

  ShmId memory_;

  uint8_t Fetch();
  void DecodeAndExecute(uint8_t opcode);

  void NOP();
  void LD_BC_nn();
  void INC_BC();
  void DEC_BC();
  void LD_B_n();
  void ADD_HL_BC();
  void LD_A_BC();
  void DEC_B();
  void JR_n();
};

}
}
