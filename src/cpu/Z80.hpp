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
};

}
}  
