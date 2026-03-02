#pragma once

#include "prs/extensions.hpp"
#include "prs/memory.hpp"
#include "prs/regs.hpp"
#include "prs/sim.hpp"

#include <unistd.h>

#include <array>
#include <bit>
#include <bitset>
#include <cassert>
#include <concepts>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <ranges>
#include <unordered_map>
#include <utility>
#include <variant>

namespace prs {

using inst_t = uint32_t;

enum class riscv_opcode_t {
  // arithmetic
  ADD,
  SUB,
  OR,
  XOR,
  AND,
  SRL,
  SLL,
  SRA,
  // arithmetic with immediate
  ADDI,
  ORI,
  XORI,
  ANDI,
  SRLI,
  SLLI,
  SRAI,
  // jumps and calls
  JAL,
  JALR,
  BEQ,
  BNE,
  BLT,
  BGE,
  BLTU,
  BGEU,
  // loads/sotres
  LB,
  LH,
  LW,
  LBU,
  LHU,
  SB,
  SH,
  SW,
  // data flow
  SLT,
  SLTU,
  SLTI,
  SLTIU,
  // upper immediate
  LUI,
  AUIPC,
  // special ones
  FENCE,
  ECALL,
  EBREAK,
  // zicsr
  CSRRW,
  CSRRS,
  CSRRC,
  CSRRWI,
  CSRRSI,
  CSRRCI,
};

struct no_funct_t final {};

inline constexpr no_funct_t no_funct;

template <size_t bitwidth, size_t offset> class funct_t final {
  inst_t data;
  bool has_funct = true;

public:
  consteval funct_t(no_funct_t) noexcept : data(0), has_funct(false) {}

  consteval funct_t(inst_t funct) noexcept {
    *this = funct_t(std::bitset<bitwidth>(funct));
  }

  consteval funct_t(std::bitset<bitwidth> funct) noexcept
      : data(funct.to_ulong() << offset) {}

  consteval operator inst_t() const noexcept { return data; }

  consteval inst_t filter() noexcept {
    if (!has_funct)
      return 0;
    return std::bitset<bitwidth>().set().to_ullong() << offset;
  }
};

using funct3 = funct_t<3, 12>;
using funct7 = funct_t<7, 25>;
using opcode_encoding_t = funct_t<7, 0>;

inline constexpr auto no_funct3 = funct3(no_funct);
inline constexpr auto no_funct7 = funct7(no_funct);

inline consteval opcode_encoding_t opcode_bits(riscv_opcode_t opcode) {
  switch (opcode) {
  case riscv_opcode_t::ADD:
  case riscv_opcode_t::SUB:
  case riscv_opcode_t::OR:
  case riscv_opcode_t::XOR:
  case riscv_opcode_t::AND:
  case riscv_opcode_t::SRL:
  case riscv_opcode_t::SLL:
  case riscv_opcode_t::SRA:
  case riscv_opcode_t::SLT:
  case riscv_opcode_t::SLTU:
    return 0b0110011u;
  case riscv_opcode_t::ADDI:
  case riscv_opcode_t::ORI:
  case riscv_opcode_t::XORI:
  case riscv_opcode_t::ANDI:
  case riscv_opcode_t::SRLI:
  case riscv_opcode_t::SLLI:
  case riscv_opcode_t::SRAI:
  case riscv_opcode_t::SLTI:
  case riscv_opcode_t::SLTIU:
    return 0b0010011u;
  case riscv_opcode_t::JAL:
    return 0b1101111u;
  case riscv_opcode_t::JALR:
    return 0b1100111u;
  case riscv_opcode_t::LB:
  case riscv_opcode_t::LH:
  case riscv_opcode_t::LW:
  case riscv_opcode_t::LBU:
  case riscv_opcode_t::LHU:
    return 0b0000011u;
  case riscv_opcode_t::ECALL:
  case riscv_opcode_t::EBREAK:
    return 0b1110011u;
  case riscv_opcode_t::FENCE:
    return 0b0001111u;
  case riscv_opcode_t::BEQ:
  case riscv_opcode_t::BNE:
  case riscv_opcode_t::BLT:
  case riscv_opcode_t::BGE:
  case riscv_opcode_t::BLTU:
  case riscv_opcode_t::BGEU:
    return 0b1100011u;
  case riscv_opcode_t::SB:
  case riscv_opcode_t::SH:
  case riscv_opcode_t::SW:
    return 0b0100011u;
  case riscv_opcode_t::LUI:
    return 0b0110111u;
  case riscv_opcode_t::AUIPC:
    return 0b0010111u;
  case riscv_opcode_t::CSRRW:
  case riscv_opcode_t::CSRRS:
  case riscv_opcode_t::CSRRC:
  case riscv_opcode_t::CSRRWI:
  case riscv_opcode_t::CSRRSI:
  case riscv_opcode_t::CSRRCI:
    return 0b1110011u;
  default:
    assert(false);
  }
};

enum class encoding_t {
  RTYPE,
  ITYPE,
  STYPE,
  BTYPE,
  UTYPE,
  JTYPE,
  SYSTYPE,
  CSRTYPE,
  CSRITYPE,
};

inline constexpr encoding_t encoding4opcode(riscv_opcode_t opcode) {
  switch (opcode) {
  case riscv_opcode_t::ADD:
  case riscv_opcode_t::SUB:
  case riscv_opcode_t::OR:
  case riscv_opcode_t::XOR:
  case riscv_opcode_t::AND:
  case riscv_opcode_t::SRL:
  case riscv_opcode_t::SLL:
  case riscv_opcode_t::SRA:
  case riscv_opcode_t::SLT:
  case riscv_opcode_t::SLTU:
    return encoding_t::RTYPE;
  case riscv_opcode_t::ADDI:
  case riscv_opcode_t::ORI:
  case riscv_opcode_t::XORI:
  case riscv_opcode_t::ANDI:
  case riscv_opcode_t::SRLI:
  case riscv_opcode_t::SLLI:
  case riscv_opcode_t::SRAI:
  case riscv_opcode_t::JALR:
  case riscv_opcode_t::LB:
  case riscv_opcode_t::LH:
  case riscv_opcode_t::LW:
  case riscv_opcode_t::LBU:
  case riscv_opcode_t::LHU:
  case riscv_opcode_t::SLTI:
  case riscv_opcode_t::SLTIU:
  case riscv_opcode_t::FENCE:
    return encoding_t::ITYPE;
  case riscv_opcode_t::JAL:
    return encoding_t::JTYPE;
  case riscv_opcode_t::BEQ:
  case riscv_opcode_t::BNE:
  case riscv_opcode_t::BLT:
  case riscv_opcode_t::BGE:
  case riscv_opcode_t::BLTU:
  case riscv_opcode_t::BGEU:
    return encoding_t::BTYPE;
  case riscv_opcode_t::SB:
  case riscv_opcode_t::SH:
  case riscv_opcode_t::SW:
    return encoding_t::STYPE;
  case riscv_opcode_t::LUI:
  case riscv_opcode_t::AUIPC:
    return encoding_t::UTYPE;
  case riscv_opcode_t::ECALL:
  case riscv_opcode_t::EBREAK:
    return encoding_t::SYSTYPE;
  case riscv_opcode_t::CSRRW:
  case riscv_opcode_t::CSRRS:
  case riscv_opcode_t::CSRRC:
    return encoding_t::CSRTYPE;
  case riscv_opcode_t::CSRRWI:
  case riscv_opcode_t::CSRRSI:
  case riscv_opcode_t::CSRRCI:
    return encoding_t::CSRITYPE;
  default:
    assert(false);
  }
}

template <class... ts_t> struct overloaded_t final : ts_t... {
  using ts_t::operator()...;
};

template <class... ts_t> auto overloaded(ts_t &&...ts) {
  return overloaded_t{std::forward<ts_t>(ts)...};
}

class reg_operand_info_t {
  bool dst;

public:
  reg_operand_info_t(bool is_dst) noexcept : dst(is_dst) {}

  auto is_dst() const noexcept { return dst; }
};
PRS_EXPECT_OPERAND_INFO_IMPL(reg_operand_info_t);

class imm_operand_info_t {
public:
  imm_operand_info_t() noexcept {}
};
PRS_EXPECT_OPERAND_INFO_IMPL(imm_operand_info_t);

class csr_operand_info_t {
public:
  csr_operand_info_t() noexcept {}
};
PRS_EXPECT_OPERAND_INFO_IMPL(csr_operand_info_t);

using riscv_operand_info_t =
    operand_info_t<reg_operand_info_t, imm_operand_info_t, csr_operand_info_t>;

class reg_operand_t final : private reg_operand_info_t {
public:
  using op_info_t = reg_operand_info_t;

private:
  size_t reg;

public:
  reg_operand_t(reg_operand_info_t op_info, size_t reg_num)
      : reg_operand_info_t(op_info), reg(reg_num) {
    assert(reg_num < reg_file_t::n_gpr_regs);
  }

  auto get_reg() const { return reg; }

  using reg_operand_info_t::is_dst;
};
PRS_EXPECT_OPERAND_IMPL(reg_operand_t);

class imm_operand_t final : private imm_operand_info_t {
public:
  using op_info_t = imm_operand_info_t;

private:
  reg_t imm_data;

public:
  imm_operand_t(imm_operand_info_t info, reg_t imm)
      : imm_operand_info_t(info), imm_data(imm) {}

  auto imm() const noexcept { return imm_data; }
};
PRS_EXPECT_OPERAND_IMPL(imm_operand_t);

enum class csr_t {};

class csr_operand_t final : private csr_operand_info_t {
public:
  using op_info_t = csr_operand_info_t;

private:
  csr_t csr_data;

public:
  csr_operand_t(csr_operand_info_t info, csr_t csr)
      : csr_operand_info_t(info), csr_data(csr) {}

  auto csr() const noexcept { return csr_data; }
};
PRS_EXPECT_OPERAND_IMPL(csr_operand_t);

using riscv_operand_t = operand_t<reg_operand_t, imm_operand_t, csr_operand_t>;

class reg_operand_matcher_t {
public:
  using inst_type = inst_t;
  using extract_type = size_t;

private:
  inst_t filter_mask;
  size_t offset;

public:
  reg_operand_matcher_t(inst_t filter, size_t offset) noexcept
      : filter_mask(filter), offset(offset) {}

  size_t extract(inst_t inst) const noexcept {
    return (inst & filter_mask) >> offset;
  }
};

class imm_operand_matcher_t {
public:
  using inst_type = inst_t;
  using extract_type = reg_t;

  enum class bit_map_t {
    TO_0 = 0,
    TO_1 = 1,
    TO_2 = 2,
    TO_3 = 3,
    TO_4 = 4,
    TO_5 = 5,
    TO_6 = 6,
    TO_7 = 7,
    TO_8 = 8,
    TO_9 = 9,
    TO_10 = 10,
    TO_11 = 11,
    TO_12 = 12,
    TO_13 = 13,
    TO_14 = 14,
    TO_15 = 15,
    TO_16 = 16,
    TO_17 = 17,
    TO_18 = 18,
    TO_19 = 19,
    TO_20 = 20,
    TO_21 = 21,
    TO_22 = 22,
    TO_23 = 23,
    TO_24 = 24,
    TO_25 = 25,
    TO_26 = 26,
    TO_27 = 27,
    TO_28 = 28,
    TO_29 = 29,
    TO_30 = 30,
    TO_31 = 31,
    ALWAYS_0,
  };

private:
  std::array<bit_map_t, num_bits_in_inst> bit_mapping;

  static bool get_bit(inst_t val, bit_map_t idx) {
    constexpr size_t bitwidth = num_bits_in_inst;
    if (idx == bit_map_t::ALWAYS_0)
      return 0;
    return std::bitset<bitwidth>(val)[static_cast<size_t>(idx)];
  }

  static reg_t set_bit(reg_t val, size_t idx, bool bit) {
    constexpr size_t bitwidth = xlen;
    std::bitset<bitwidth> bitval(std::bit_cast<ureg_t>(val));
    bitval[static_cast<size_t>(idx)] = bit;
    ureg_t uval = bitval.to_ulong();
    return std::bit_cast<reg_t>(uval);
  }

public:
  imm_operand_matcher_t(std::array<bit_map_t, num_bits_in_inst> mapping)
      : bit_mapping{std::move(mapping)} {}

  reg_t extract(inst_t inst) const {
    reg_t imm = 0;
    for (auto [tgt_idx, src_idx] : bit_mapping | std::views::enumerate)
      imm = set_bit(imm, tgt_idx, get_bit(inst, src_idx));
    return imm;
  }

  static const std::unordered_map<encoding_t, imm_operand_matcher_t> per_enc;
};

inline const std::unordered_map<encoding_t, imm_operand_matcher_t>
    imm_operand_matcher_t::per_enc = {
        {encoding_t::ITYPE,
         std::array{bit_map_t::TO_20, bit_map_t::TO_21, bit_map_t::TO_22,
                    bit_map_t::TO_23, bit_map_t::TO_24, bit_map_t::TO_25,
                    bit_map_t::TO_26, bit_map_t::TO_27, bit_map_t::TO_28,
                    bit_map_t::TO_29, bit_map_t::TO_30, bit_map_t::TO_31,
                    bit_map_t::TO_31, bit_map_t::TO_31, bit_map_t::TO_31,
                    bit_map_t::TO_31, bit_map_t::TO_31, bit_map_t::TO_31,
                    bit_map_t::TO_31, bit_map_t::TO_31, bit_map_t::TO_31,
                    bit_map_t::TO_31, bit_map_t::TO_31, bit_map_t::TO_31,
                    bit_map_t::TO_31, bit_map_t::TO_31, bit_map_t::TO_31,
                    bit_map_t::TO_31, bit_map_t::TO_31, bit_map_t::TO_31,
                    bit_map_t::TO_31, bit_map_t::TO_31}},
        {encoding_t::STYPE,
         std::array{bit_map_t::TO_7,  bit_map_t::TO_8,  bit_map_t::TO_9,
                    bit_map_t::TO_10, bit_map_t::TO_11, bit_map_t::TO_25,
                    bit_map_t::TO_26, bit_map_t::TO_27, bit_map_t::TO_28,
                    bit_map_t::TO_29, bit_map_t::TO_30, bit_map_t::TO_31,
                    bit_map_t::TO_31, bit_map_t::TO_31, bit_map_t::TO_31,
                    bit_map_t::TO_31, bit_map_t::TO_31, bit_map_t::TO_31,
                    bit_map_t::TO_31, bit_map_t::TO_31, bit_map_t::TO_31,
                    bit_map_t::TO_31, bit_map_t::TO_31, bit_map_t::TO_31,
                    bit_map_t::TO_31, bit_map_t::TO_31, bit_map_t::TO_31,
                    bit_map_t::TO_31, bit_map_t::TO_31, bit_map_t::TO_31,
                    bit_map_t::TO_31, bit_map_t::TO_31}},
        {encoding_t::BTYPE,
         std::array{bit_map_t::ALWAYS_0, bit_map_t::TO_8,  bit_map_t::TO_9,
                    bit_map_t::TO_10,    bit_map_t::TO_11, bit_map_t::TO_25,
                    bit_map_t::TO_26,    bit_map_t::TO_27, bit_map_t::TO_28,
                    bit_map_t::TO_29,    bit_map_t::TO_30, bit_map_t::TO_7,
                    bit_map_t::TO_31,    bit_map_t::TO_31, bit_map_t::TO_31,
                    bit_map_t::TO_31,    bit_map_t::TO_31, bit_map_t::TO_31,
                    bit_map_t::TO_31,    bit_map_t::TO_31, bit_map_t::TO_31,
                    bit_map_t::TO_31,    bit_map_t::TO_31, bit_map_t::TO_31,
                    bit_map_t::TO_31,    bit_map_t::TO_31, bit_map_t::TO_31,
                    bit_map_t::TO_31,    bit_map_t::TO_31, bit_map_t::TO_31,
                    bit_map_t::TO_31,    bit_map_t::TO_31}},
        {encoding_t::UTYPE,
         std::array{
             bit_map_t::ALWAYS_0, bit_map_t::ALWAYS_0, bit_map_t::ALWAYS_0,
             bit_map_t::ALWAYS_0, bit_map_t::ALWAYS_0, bit_map_t::ALWAYS_0,
             bit_map_t::ALWAYS_0, bit_map_t::ALWAYS_0, bit_map_t::ALWAYS_0,
             bit_map_t::ALWAYS_0, bit_map_t::ALWAYS_0, bit_map_t::ALWAYS_0,
             bit_map_t::TO_12,    bit_map_t::TO_13,    bit_map_t::TO_14,
             bit_map_t::TO_15,    bit_map_t::TO_16,    bit_map_t::TO_17,
             bit_map_t::TO_18,    bit_map_t::TO_19,    bit_map_t::TO_20,
             bit_map_t::TO_21,    bit_map_t::TO_22,    bit_map_t::TO_23,
             bit_map_t::TO_24,    bit_map_t::TO_25,    bit_map_t::TO_26,
             bit_map_t::TO_27,    bit_map_t::TO_28,    bit_map_t::TO_29,
             bit_map_t::TO_30,    bit_map_t::TO_31}},
        {encoding_t::JTYPE,
         std::array{bit_map_t::ALWAYS_0, bit_map_t::TO_21, bit_map_t::TO_22,
                    bit_map_t::TO_23,    bit_map_t::TO_24, bit_map_t::TO_25,
                    bit_map_t::TO_26,    bit_map_t::TO_27, bit_map_t::TO_28,
                    bit_map_t::TO_29,    bit_map_t::TO_30, bit_map_t::TO_20,
                    bit_map_t::TO_12,    bit_map_t::TO_13, bit_map_t::TO_14,
                    bit_map_t::TO_15,    bit_map_t::TO_16, bit_map_t::TO_17,
                    bit_map_t::TO_18,    bit_map_t::TO_19, bit_map_t::TO_31,
                    bit_map_t::TO_31,    bit_map_t::TO_31, bit_map_t::TO_31,
                    bit_map_t::TO_31,    bit_map_t::TO_31, bit_map_t::TO_31,
                    bit_map_t::TO_31,    bit_map_t::TO_31, bit_map_t::TO_31,
                    bit_map_t::TO_31,    bit_map_t::TO_31}},
        {encoding_t::CSRITYPE,
         std::array{
             bit_map_t::TO_0,     bit_map_t::TO_1,     bit_map_t::TO_2,
             bit_map_t::TO_3,     bit_map_t::TO_4,     bit_map_t::ALWAYS_0,
             bit_map_t::ALWAYS_0, bit_map_t::ALWAYS_0, bit_map_t::ALWAYS_0,
             bit_map_t::ALWAYS_0, bit_map_t::ALWAYS_0, bit_map_t::ALWAYS_0,
             bit_map_t::ALWAYS_0, bit_map_t::ALWAYS_0, bit_map_t::ALWAYS_0,
             bit_map_t::ALWAYS_0, bit_map_t::ALWAYS_0, bit_map_t::ALWAYS_0,
             bit_map_t::ALWAYS_0, bit_map_t::ALWAYS_0, bit_map_t::ALWAYS_0,
             bit_map_t::ALWAYS_0, bit_map_t::ALWAYS_0, bit_map_t::ALWAYS_0,
             bit_map_t::ALWAYS_0, bit_map_t::ALWAYS_0, bit_map_t::ALWAYS_0,
             bit_map_t::ALWAYS_0, bit_map_t::ALWAYS_0, bit_map_t::ALWAYS_0,
             bit_map_t::ALWAYS_0, bit_map_t::ALWAYS_0}},
};

class csr_operand_matcher_t {
public:
  using inst_type = inst_t;
  using extract_type = csr_t;

  consteval csr_operand_matcher_t() noexcept {}

  csr_t extract(inst_t inst) const noexcept {
    constexpr size_t bitwidth = 12;
    constexpr size_t offset = 20;
    constexpr inst_t filter_mask = std::bitset<bitwidth>().set().to_ullong()
                                   << offset;

    return static_cast<csr_t>((inst & filter_mask) >> offset);
  }
};

PRS_EXPECT_OPERAND_MATCHER_IMPL(reg_operand_matcher_t);
PRS_EXPECT_OPERAND_MATCHER_IMPL(imm_operand_matcher_t);
PRS_EXPECT_OPERAND_MATCHER_IMPL(csr_operand_matcher_t);

using riscv_operand_matcher_t =
    operand_matcher_t<reg_operand_matcher_t, imm_operand_matcher_t,
                      csr_operand_matcher_t>;

#define PRS_RISCV_MAP_OP_INFO_TO_OP(op_info, op, matcher)                      \
  template <> struct op_info_traits<op_info> {                                 \
    using op_type = op;                                                        \
    using matcher_type = matcher;                                              \
  }

PRS_RISCV_MAP_OP_INFO_TO_OP(reg_operand_info_t, reg_operand_t,
                            reg_operand_matcher_t);
PRS_RISCV_MAP_OP_INFO_TO_OP(imm_operand_info_t, imm_operand_t,
                            imm_operand_matcher_t);
PRS_RISCV_MAP_OP_INFO_TO_OP(csr_operand_info_t, csr_operand_t,
                            csr_operand_matcher_t);

struct riscv_operand_description_tr {
  using op_info_t = riscv_operand_info_t;
  using op_t = riscv_operand_t;
  using op_matcher_t = riscv_operand_matcher_t;
};

using riscv_operand_description_t =
    operand_description_t<riscv_operand_description_tr>;

template <>
inline const typename riscv_operand_description_t::per_enc_t
    riscv_operand_description_t::per_enc = {
        {static_cast<encoding_id_t>(encoding_t::RTYPE),
         {
             {reg_operand_info_t(true), reg_operand_matcher_t(0x00000f80, 7)},
             {reg_operand_info_t(false), reg_operand_matcher_t(0x000f8000, 15)},
             {reg_operand_info_t(false), reg_operand_matcher_t(0x01f00000, 20)},
         }},
        {static_cast<encoding_id_t>(encoding_t::ITYPE),
         {
             {reg_operand_info_t(true), reg_operand_matcher_t(0x00000f80, 7)},
             {reg_operand_info_t(false), reg_operand_matcher_t(0x000f8000, 15)},
             {imm_operand_info_t(),
              imm_operand_matcher_t::per_enc.at(encoding_t::ITYPE)},
         }},
        {static_cast<encoding_id_t>(encoding_t::STYPE),
         {
             {reg_operand_info_t(false), reg_operand_matcher_t(0x000f8000, 15)},
             {reg_operand_info_t(false), reg_operand_matcher_t(0x01f00000, 20)},
             {imm_operand_info_t(),
              imm_operand_matcher_t::per_enc.at(encoding_t::STYPE)},
         }},
        {static_cast<encoding_id_t>(encoding_t::BTYPE),
         {
             {reg_operand_info_t(false), reg_operand_matcher_t(0x000f8000, 15)},
             {reg_operand_info_t(false), reg_operand_matcher_t(0x01f00000, 20)},
             {imm_operand_info_t(),
              imm_operand_matcher_t::per_enc.at(encoding_t::BTYPE)},
         }},
        {static_cast<encoding_id_t>(encoding_t::UTYPE),
         {
             {reg_operand_info_t(true), reg_operand_matcher_t(0x00000f80, 7)},
             {imm_operand_info_t(),
              imm_operand_matcher_t::per_enc.at(encoding_t::UTYPE)},
         }},
        {static_cast<encoding_id_t>(encoding_t::JTYPE),
         {
             {reg_operand_info_t(true), reg_operand_matcher_t(0x00000f80, 7)},
             {imm_operand_info_t(),
              imm_operand_matcher_t::per_enc.at(encoding_t::JTYPE)},
         }},
        {static_cast<encoding_id_t>(encoding_t::SYSTYPE), {}},
        {static_cast<encoding_id_t>(encoding_t::CSRTYPE),
         {
             {reg_operand_info_t(true), reg_operand_matcher_t(0x00000f80, 7)},
             {reg_operand_info_t(false), reg_operand_matcher_t(0x000f8000, 15)},
             {csr_operand_info_t(), csr_operand_matcher_t()},
         }},
        {static_cast<encoding_id_t>(encoding_t::CSRITYPE),
         {
             {reg_operand_info_t(true), reg_operand_matcher_t(0x00000f80, 7)},
             {imm_operand_info_t(),
              imm_operand_matcher_t::per_enc.at(encoding_t::CSRITYPE)},
             {csr_operand_info_t(), csr_operand_matcher_t()},
         }},
};

inline std::vector<riscv_operand_description_t>
descriptions4encoding(encoding_t enc) {
  auto &descs = riscv_operand_description_t::per_enc;
  auto it = descs.find(static_cast<encoding_id_t>(enc));
  assert(it != descs.end());
  return it->second;
}

template <> struct opcode_traits<riscv_opcode_t>;

using riscv_decoded_inst_t = decoded_inst_t<riscv_opcode_t, riscv_operand_t>;

template <> struct op_traits<riscv_operand_t> {
  static auto print_visitor(auto &&l) {
    return overloaded([&](reg_operand_t reg) { l << "\tx" << reg.get_reg(); },
                      [&](imm_operand_t imm) { l << '\t' << imm.imm(); },
                      [&](csr_operand_t csr) { l << "\tCSR"; });
  }
};

class riscv_opcode_matcher_t final {
  inst_t filter_mask;
  inst_t match_mask;

public:
  consteval riscv_opcode_matcher_t(inst_t inst) noexcept
      : filter_mask(~static_cast<inst_t>(0)), match_mask(inst) {}

  consteval riscv_opcode_matcher_t(opcode_encoding_t opcode, funct3 f3,
                                   funct7 f7) noexcept
      : filter_mask(opcode.filter() | f3.filter() | f7.filter()),
        match_mask(opcode | f3 | f7) {}

  constexpr bool match(inst_t inst) const noexcept {
    return (inst & filter_mask) == match_mask;
  }

  static const std::unordered_map<riscv_opcode_t, riscv_opcode_matcher_t>
      per_opc;
};

static_assert(opcode_matcher_c<riscv_opcode_matcher_t, inst_t>);

inline const std::unordered_map<riscv_opcode_t, riscv_opcode_matcher_t>
    riscv_opcode_matcher_t::per_opc = {
#define PRS_REGISTER_OPCODE(opc, f3, f7)                                       \
  {riscv_opcode_t::opc,                                                        \
   riscv_opcode_matcher_t(opcode_bits(riscv_opcode_t::opc), (f3), (f7))}

        // arithmetic
        PRS_REGISTER_OPCODE(ADD, 0b000u, 0b0000000u),
        PRS_REGISTER_OPCODE(SUB, 0b000u, 0b0100000u),
        PRS_REGISTER_OPCODE(OR, 0b110u, 0b0000000u),
        PRS_REGISTER_OPCODE(XOR, 0b100u, 0b0000000u),
        PRS_REGISTER_OPCODE(AND, 0b111u, 0b0000000u),
        PRS_REGISTER_OPCODE(SRL, 0b101u, 0b0000000u),
        PRS_REGISTER_OPCODE(SLL, 0b001u, 0b0000000u),
        PRS_REGISTER_OPCODE(SRA, 0b101u, 0b0100000u),
        // arithmetic with immediate
        PRS_REGISTER_OPCODE(ADDI, 0b000u, no_funct7),
        PRS_REGISTER_OPCODE(ORI, 0b110u, no_funct7),
        PRS_REGISTER_OPCODE(XORI, 0b100u, no_funct7),
        PRS_REGISTER_OPCODE(ANDI, 0b111u, no_funct7),
        PRS_REGISTER_OPCODE(SRLI, 0b101u, no_funct7),
        PRS_REGISTER_OPCODE(SLLI, 0b001u, no_funct7),
        PRS_REGISTER_OPCODE(SRAI, 0b101u, no_funct7),
        // jumps and calls
        PRS_REGISTER_OPCODE(JAL, no_funct3, no_funct7),
        PRS_REGISTER_OPCODE(JALR, 0b000u, no_funct7),
        PRS_REGISTER_OPCODE(BEQ, 0b000u, no_funct7),
        PRS_REGISTER_OPCODE(BNE, 0b001u, no_funct7),
        PRS_REGISTER_OPCODE(BLT, 0b100u, no_funct7),
        PRS_REGISTER_OPCODE(BGE, 0b101u, no_funct7),
        PRS_REGISTER_OPCODE(BLTU, 0b110u, no_funct7),
        PRS_REGISTER_OPCODE(BGEU, 0b111u, no_funct7),
        // loads/sotres
        PRS_REGISTER_OPCODE(LB, 0b000u, no_funct7),
        PRS_REGISTER_OPCODE(LH, 0b001u, no_funct7),
        PRS_REGISTER_OPCODE(LW, 0b010u, no_funct7),
        PRS_REGISTER_OPCODE(LBU, 0b100u, no_funct7),
        PRS_REGISTER_OPCODE(LHU, 0b101u, no_funct7),
        PRS_REGISTER_OPCODE(SB, 0b000u, no_funct7),
        PRS_REGISTER_OPCODE(SH, 0b001u, no_funct7),
        PRS_REGISTER_OPCODE(SW, 0b010u, no_funct7),
        // data flow
        PRS_REGISTER_OPCODE(SLT, 0b010u, 0b0000000u),
        PRS_REGISTER_OPCODE(SLTU, 0b011u, 0b0000000u),
        PRS_REGISTER_OPCODE(SLTI, 0b010u, no_funct7),
        PRS_REGISTER_OPCODE(SLTIU, 0b100u, no_funct7),
        // upper immediate
        PRS_REGISTER_OPCODE(LUI, no_funct3, no_funct7),
        PRS_REGISTER_OPCODE(AUIPC, no_funct3, no_funct7),
        // special ones
        PRS_REGISTER_OPCODE(FENCE, 0b010u, 0b0000000u),
        {riscv_opcode_t::ECALL, riscv_opcode_matcher_t(0b1110011)},
        {riscv_opcode_t::EBREAK,
         riscv_opcode_matcher_t(0b00000000000100000000000001110011)},
        // zicsr
        PRS_REGISTER_OPCODE(CSRRW, 0b001u, no_funct7),
        PRS_REGISTER_OPCODE(CSRRS, 0b010u, no_funct7),
        PRS_REGISTER_OPCODE(CSRRC, 0b011u, no_funct7),
        PRS_REGISTER_OPCODE(CSRRWI, 0b101u, no_funct7),
        PRS_REGISTER_OPCODE(CSRRSI, 0b110u, no_funct7),
        PRS_REGISTER_OPCODE(CSRRCI, 0b111u, no_funct7),
#undef PRS_REGISTER_OPCODE
};

template <> struct opcode_traits<riscv_opcode_t> {
  static riscv_opcode_matcher_t matcher(riscv_opcode_t opcode) {
    auto &matchers = riscv_opcode_matcher_t::per_opc;
    auto it = matchers.find(opcode);
    assert(it != matchers.end());
    return it->second;
  }

  static auto descriptions(riscv_opcode_t opcode) {
    return descriptions4encoding(encoding4opcode(opcode));
  }

  static constexpr std::array opcodes = {
      // arithmetic
      riscv_opcode_t::ADD,
      riscv_opcode_t::SUB,
      riscv_opcode_t::OR,
      riscv_opcode_t::XOR,
      riscv_opcode_t::AND,
      riscv_opcode_t::SRL,
      riscv_opcode_t::SLL,
      riscv_opcode_t::SRA,
      // arithmetic with immediate
      riscv_opcode_t::ADDI,
      riscv_opcode_t::ORI,
      riscv_opcode_t::XORI,
      riscv_opcode_t::ANDI,
      riscv_opcode_t::SRLI,
      riscv_opcode_t::SLLI,
      riscv_opcode_t::SRAI,
      // jumps and calls
      riscv_opcode_t::JAL,
      riscv_opcode_t::JALR,
      riscv_opcode_t::BEQ,
      riscv_opcode_t::BNE,
      riscv_opcode_t::BLT,
      riscv_opcode_t::BGE,
      riscv_opcode_t::BLTU,
      riscv_opcode_t::BGEU,
      // loads/sotres
      riscv_opcode_t::LB,
      riscv_opcode_t::LH,
      riscv_opcode_t::LW,
      riscv_opcode_t::LBU,
      riscv_opcode_t::LHU,
      riscv_opcode_t::SB,
      riscv_opcode_t::SH,
      riscv_opcode_t::SW,
      // data flow
      riscv_opcode_t::SLT,
      riscv_opcode_t::SLTU,
      riscv_opcode_t::SLTI,
      riscv_opcode_t::SLTIU,
      // upper immediate
      riscv_opcode_t::LUI,
      riscv_opcode_t::AUIPC,
      // special ones
      riscv_opcode_t::FENCE,
      riscv_opcode_t::ECALL,
      riscv_opcode_t::EBREAK,
      // zicsr
      riscv_opcode_t::CSRRW,
      riscv_opcode_t::CSRRS,
      riscv_opcode_t::CSRRC,
      riscv_opcode_t::CSRRWI,
      riscv_opcode_t::CSRRSI,
      riscv_opcode_t::CSRRCI,
  };

  static constexpr std::string_view to_str(riscv_opcode_t opc) {
    switch (opc) {
#define OPC_TO_STR_CASE(opcode)                                                \
  case riscv_opcode_t::opcode:                                                 \
    return #opcode

      OPC_TO_STR_CASE(ADD);
      OPC_TO_STR_CASE(SUB);
      OPC_TO_STR_CASE(OR);
      OPC_TO_STR_CASE(XOR);
      OPC_TO_STR_CASE(AND);
      OPC_TO_STR_CASE(SRL);
      OPC_TO_STR_CASE(SLL);
      OPC_TO_STR_CASE(SRA);
      OPC_TO_STR_CASE(ADDI);
      OPC_TO_STR_CASE(ORI);
      OPC_TO_STR_CASE(XORI);
      OPC_TO_STR_CASE(ANDI);
      OPC_TO_STR_CASE(SRLI);
      OPC_TO_STR_CASE(SLLI);
      OPC_TO_STR_CASE(SRAI);
      OPC_TO_STR_CASE(JAL);
      OPC_TO_STR_CASE(JALR);
      OPC_TO_STR_CASE(BEQ);
      OPC_TO_STR_CASE(BNE);
      OPC_TO_STR_CASE(BLT);
      OPC_TO_STR_CASE(BGE);
      OPC_TO_STR_CASE(BLTU);
      OPC_TO_STR_CASE(BGEU);
      OPC_TO_STR_CASE(LB);
      OPC_TO_STR_CASE(LH);
      OPC_TO_STR_CASE(LW);
      OPC_TO_STR_CASE(LBU);
      OPC_TO_STR_CASE(LHU);
      OPC_TO_STR_CASE(SB);
      OPC_TO_STR_CASE(SH);
      OPC_TO_STR_CASE(SW);
      OPC_TO_STR_CASE(SLT);
      OPC_TO_STR_CASE(SLTU);
      OPC_TO_STR_CASE(SLTI);
      OPC_TO_STR_CASE(SLTIU);
      OPC_TO_STR_CASE(LUI);
      OPC_TO_STR_CASE(AUIPC);
      OPC_TO_STR_CASE(FENCE);
      OPC_TO_STR_CASE(ECALL);
      OPC_TO_STR_CASE(EBREAK);
      OPC_TO_STR_CASE(CSRRW);
      OPC_TO_STR_CASE(CSRRS);
      OPC_TO_STR_CASE(CSRRC);
      OPC_TO_STR_CASE(CSRRWI);
      OPC_TO_STR_CASE(CSRRSI);
      OPC_TO_STR_CASE(CSRRCI);

#undef OPC_TO_STR_CASE
    }
    assert(false);
  }
};

using riscv_inst_description_t =
    inst_description_t<riscv_opcode_t, inst_t, riscv_opcode_matcher_t,
                       riscv_operand_description_t>;

using riscv_inst_info_t = inst_info_t<riscv_inst_description_t>;

inline const riscv_inst_info_t inst_info;

inline auto decode(inst_t inst) {
  auto it = std::ranges::find_if(
      inst_info, [inst](const auto &desc) { return desc.match_opc(inst); });
  assert(it != inst_info.end());
  return riscv_decoded_inst_t(it->opcode(), it->operands(inst));
}

enum class mem_op_size_t {
  BYTE,
  HALF,
  WORD,
  UBYTE,
  UHALF,
};

enum class syscall_num_t {
  CLOSE = 57,
  WRITE = 64,
  NEWFSTAT = 80,
  EXIT = 93,
  BRK = 214,
};

enum class trap_cause_t {
  NONE,
  ECALL,
  EBREAK,
};

namespace uop {
static constexpr auto add_op = std::plus{};
static constexpr auto sub_op = std::minus{};
static constexpr auto or_op = std::bit_or{};
static constexpr auto xor_op = std::bit_xor{};
static constexpr auto and_op = std::bit_and{};
static constexpr auto srl_op = [](reg_t s1, reg_t s2) {
  return to_ureg(s1) >> s2;
};
static constexpr auto sll_op = [](reg_t s1, reg_t s2) {
  return to_ureg(s1) << s2;
};
static constexpr auto sra_op = [](reg_t s1, reg_t s2) { return s1 >> s2; };
static constexpr auto slt_op = [](reg_t s1, reg_t s2) {
  return s1 < s2 ? 1 : 0;
};
static constexpr auto sltu_op = [](reg_t s1, reg_t s2) {
  return to_ureg(s1) < to_ureg(s2) ? 1 : 0;
};

static constexpr auto eq_op = std::equal_to{};
static constexpr auto ne_op = std::not_equal_to{};
static constexpr auto lt_op = std::less{};
static constexpr auto ge_op = std::greater_equal{};
static constexpr auto ltu_op = [](reg_t s1, reg_t s2) {
  return lt_op(to_ureg(s1), to_ureg(s2));
};
static constexpr auto geu_op = [](reg_t s1, reg_t s2) {
  return ge_op(to_ureg(s1), to_ureg(s2));
};
} // namespace uop

class riscv_simulator_t {
  memory_t memory;
  reg_file_t regs;

  trap_cause_t trap_cause = trap_cause_t::NONE;

  logger_t logger;

  static constexpr size_t rd_idx = 0;
  static constexpr size_t rs1_idx = 1;
  static constexpr size_t rs2_idx = 2;
  static constexpr size_t rs1_no_rd_idx = 0;
  static constexpr size_t rs2_no_rd_idx = 1;

  static constexpr bool dst = true;
  static constexpr bool src = false;

  template <bool is_dst = src> static size_t extract_reg(riscv_operand_t op) {
    size_t reg;
    op.visit(
        [&reg](reg_operand_t reg_op) {
          assert(reg_op.is_dst() == is_dst);
          reg = reg_op.get_reg();
        },
        [](auto other_ops) { assert(false); });
    return reg;
  }

  static reg_t extract_imm(riscv_operand_t op) {
    reg_t imm;
    op.visit([&imm](imm_operand_t imm_op) { imm = imm_op.imm(); },
             [](auto other_ops) { assert(false); });
    return imm;
  }

  void exec_reg_arithmetic(std::span<const riscv_operand_t> ops,
                           std::invocable<reg_t, reg_t> auto &&operation) {
    assert(ops.size() == 3);
    auto rd_op = ops[rd_idx];
    auto rs1_op = ops[rs1_idx];
    auto rs2_op = ops[rs2_idx];
    auto rd = extract_reg<dst>(rd_op);
    auto rs1 = extract_reg(rs1_op);
    auto rs2 = extract_reg(rs2_op);
    auto rs1_val = regs.read(rs1);
    auto rs2_val = regs.read(rs2);
    auto rd_val = operation(rs1_val, rs2_val);
    regs.write(rd, rd_val);
    regs.set_pc(regs.get_pc() + pc_step);
  }

  void exec_imm_arithmetic(std::span<const riscv_operand_t> ops,
                           std::invocable<reg_t, reg_t> auto &&operation) {
    assert(ops.size() == 3);
    auto rd_op = ops[rd_idx];
    auto rs1_op = ops[rs1_idx];
    constexpr size_t imm_idx = 2;
    auto imm_op = ops[imm_idx];
    auto rd = extract_reg<dst>(rd_op);
    auto rs1 = extract_reg(rs1_op);
    auto imm = extract_imm(imm_op);
    auto rs1_val = regs.read(rs1);
    auto rd_val = operation(rs1_val, imm);
    regs.write(rd, rd_val);
    regs.set_pc(regs.get_pc() + pc_step);
  }

  reg_t load(addr_t addr, mem_op_size_t size) {
    reg_t data;
    switch (size) {
    default:
      assert(false);
    case mem_op_size_t::BYTE:
      data = static_cast<reg_t>(static_cast<byte_t>(memory.read_byte(addr)));
      break;
    case mem_op_size_t::HALF:
      data = static_cast<reg_t>(static_cast<half_t>(memory.read_half(addr)));
      break;
    case mem_op_size_t::WORD:
      data = static_cast<reg_t>(static_cast<word_t>(memory.read_word(addr)));
      break;
    case mem_op_size_t::UBYTE:
      data = to_reg(static_cast<ureg_t>(memory.read_byte(addr)));
      break;
    case mem_op_size_t::UHALF:
      data = to_reg(static_cast<ureg_t>(memory.read_half(addr)));
      break;
    }
    return data;
  }

  void exec_load(std::span<const riscv_operand_t> ops, mem_op_size_t size) {
    assert(ops.size() == 3);
    auto rd_op = ops[rd_idx];
    auto rs1_op = ops[rs1_idx];
    constexpr size_t imm_idx = 2;
    auto imm_op = ops[imm_idx];
    auto rd = extract_reg<dst>(rd_op);
    auto rs1 = extract_reg(rs1_op);
    auto imm = extract_imm(imm_op);
    auto addr = to_addr(regs.read(rs1) + imm);
    auto rd_val = load(addr, size);
    regs.write(rd, rd_val);
    regs.set_pc(regs.get_pc() + pc_step);
  }

  void store(addr_t addr, reg_t data, mem_op_size_t size) {
    switch (size) {
    default:
      assert(false);
    case mem_op_size_t::BYTE:
      memory.write_byte(addr, static_cast<ubyte_t>(data & ~ubyte_t{0}));
      break;
    case mem_op_size_t::HALF:
      memory.write_half(addr, static_cast<uhalf_t>(data & ~uhalf_t{0}));
      break;
    case mem_op_size_t::WORD:
      memory.write_word(addr, static_cast<uword_t>(data & ~uword_t{0}));
      break;
    }
  }

  void exec_store(std::span<const riscv_operand_t> ops, mem_op_size_t size) {
    assert(ops.size() == 3);
    auto rs1_op = ops[rs1_no_rd_idx];
    auto rs2_op = ops[rs2_no_rd_idx];
    constexpr size_t imm_idx = 2;
    auto imm_op = ops[imm_idx];
    auto rs1 = extract_reg(rs1_op);
    auto rs2 = extract_reg(rs2_op);
    auto imm = extract_imm(imm_op);
    auto addr = to_addr(regs.read(rs1) + imm);
    store(addr, regs.read(rs2), size);
    regs.set_pc(regs.get_pc() + pc_step);
  }

  void exec_branch(std::span<const riscv_operand_t> ops,
                   std::invocable<reg_t, reg_t> auto &&comparator) {
    assert(ops.size() == 3);
    auto rs1_op = ops[rs1_no_rd_idx];
    auto rs2_op = ops[rs2_no_rd_idx];
    constexpr size_t imm_idx = 2;
    auto imm_op = ops[imm_idx];
    auto rs1 = extract_reg(rs1_op);
    auto rs2 = extract_reg(rs2_op);
    auto imm = extract_imm(imm_op);
    auto rs1_val = regs.read(rs1);
    auto rs2_val = regs.read(rs2);
    auto taken = comparator(rs1_val, rs2_val);
    auto offset = taken ? imm : pc_step;
    regs.set_pc(regs.get_pc() + offset);
  }

  void exec_jal(std::span<const riscv_operand_t> ops) {
    assert(ops.size() == 2);
    auto rd_op = ops[rd_idx];
    constexpr size_t imm_idx = 1;
    auto imm_op = ops[imm_idx];
    auto rd = extract_reg<dst>(rd_op);
    auto imm = extract_imm(imm_op);
    auto rd_val = regs.get_pc() + pc_step;
    regs.write(rd, rd_val);
    regs.set_pc(regs.get_pc() + imm);
  }

  void exec_jalr(std::span<const riscv_operand_t> ops) {
    assert(ops.size() == 3);
    auto rd_op = ops[rd_idx];
    auto rs1_op = ops[rs1_idx];
    constexpr size_t imm_idx = 2;
    auto imm_op = ops[imm_idx];
    auto rd = extract_reg<dst>(rd_op);
    auto rs1 = extract_reg(rs1_op);
    auto imm = extract_imm(imm_op);
    auto rd_val = regs.get_pc() + pc_step;
    regs.write(rd, rd_val);
    auto rs1_val = regs.read(rs1);
    regs.set_pc(rs1_val + imm);
  }

  std::unordered_map<int, int> open_fds = {{0, 0}, {1, 1}, {2, 2}};

  void process_close() {
    auto guest_fd = regs.read(riscv_abi_reg_t::a0);
    auto fd_pos = open_fds.find(guest_fd);
    if (fd_pos == open_fds.end()) {
      regs.write(riscv_abi_reg_t::a0, -1);
      return;
    }
    open_fds.erase(fd_pos);
    regs.write(riscv_abi_reg_t::a0, 0);
  }

  void process_write() {
    auto guest_fd = regs.read(riscv_abi_reg_t::a0);
    auto host_fd = open_fds.at(guest_fd);
    auto addr = to_addr(regs.read(riscv_abi_reg_t::a1));
    auto len = regs.read(riscv_abi_reg_t::a2);
    auto written = 0;
    for (auto i : std::views::iota(0, len)) {
      char data = memory.read_byte(addr + i);
      written += write(host_fd, &data, 1);
    }
    regs.write(riscv_abi_reg_t::a0, written);
  }

  void process_newfstat() {
    regs.write(riscv_abi_reg_t::a0, -1); // unimplemented
  }

  void process_exit() {
    auto ret_val = regs.read(riscv_abi_reg_t::a0);
    std::cerr << "`exit` called. Exiting." << std::endl;
    exit(ret_val);
  }

  void process_brk() {
    auto addr = to_addr(regs.read(riscv_abi_reg_t::a0));
    memory.extend_upto(addr);
    regs.write(riscv_abi_reg_t::a0, 0);
  }

  void exec_ecall(std::span<const riscv_operand_t> ops) {
    assert(ops.empty());
    auto syscall_num = regs.read(riscv_abi_reg_t::a7);
    switch (static_cast<syscall_num_t>(syscall_num)) {
    default:
      std::cerr << "Unsupported ecall: " << syscall_num << std::endl;
      assert(false);
    case syscall_num_t::CLOSE:
      process_close();
      break;
    case syscall_num_t::WRITE:
      process_write();
      break;
    case syscall_num_t::NEWFSTAT:
      process_newfstat();
      break;
    case syscall_num_t::EXIT:
      process_exit();
      break;
    case syscall_num_t::BRK:
      process_brk();
      break;
    }
    regs.set_pc(regs.get_pc() + pc_step);
  }

  void exec_ebreak(std::span<const riscv_operand_t> ops) {
    assert(ops.empty());
    trap_cause = trap_cause_t::EBREAK;
  }

  void exec_fence(std::span<const riscv_operand_t>) {
    regs.set_pc(regs.get_pc() + pc_step);
  }

  void exec_lui(std::span<const riscv_operand_t> ops) {
    assert(ops.size() == 2);
    auto rd_op = ops[rd_idx];
    constexpr size_t imm_idx = 1;
    auto imm_op = ops[imm_idx];
    auto rd = extract_reg<dst>(rd_op);
    auto imm = extract_imm(imm_op);
    regs.write(rd, imm);
    regs.set_pc(regs.get_pc() + pc_step);
  }

  void exec_auipc(std::span<const riscv_operand_t> ops) {
    assert(ops.size() == 2);
    auto rd_op = ops[rd_idx];
    constexpr size_t imm_idx = 1;
    auto imm_op = ops[imm_idx];
    auto rd = extract_reg<dst>(rd_op);
    auto imm = extract_imm(imm_op);
    regs.write(rd, regs.get_pc() + imm);
    regs.set_pc(regs.get_pc() + pc_step);
  }

  void log_inst(inst_t inst) {
    logger << "0x" << std::hex << std::setw(8) << std::setfill('0')
           << regs.get_pc() << ":\t0x" << std::hex << std::setw(8)
           << std::setfill('0') << inst << "\n"
           << std::dec;
  }

  void log_decoded_inst(const riscv_decoded_inst_t &inst) {
    inst.print(logger);
  }

  const static std::unordered_map<
      riscv_opcode_t,
      void (*)(riscv_simulator_t &, std::span<const prs::riscv_operand_t>)>
      i_opc_actions;

  const static std::unordered_map<
      riscv_opcode_t,
      void (*)(riscv_simulator_t &, std::span<const prs::riscv_operand_t>)>
      c_opc_actions;

  const static std::unordered_map<
      riscv_opcode_t,
      void (*)(riscv_simulator_t &, std::span<const prs::riscv_operand_t>)>
      m_opc_actions;

public:
  riscv_simulator_t(enabled_extensions_t en_exts, logger_t l)
      : memory(l), regs(en_exts, l), logger(l) {}

  inst_t fetch_inst() { return memory.read_word(regs.get_pc()); }

  void exec_one() {
    auto inst = fetch_inst();
    log_inst(inst);
    auto decoded_inst = decode(inst);
    log_decoded_inst(decoded_inst);
    auto action_pos = i_opc_actions.find(decoded_inst.opc());
    assert(action_pos != i_opc_actions.end());
    action_pos->second(*this, decoded_inst.ops());
  }

  void exec_from(addr_t start) {
    regs.set_pc(start);
    for (;;) {
      if (trap_cause == trap_cause_t::EBREAK)
        break;
      exec_one();
    }
  }

  void load_section(const char *data, size_t size, addr_t addr) {
    memory.load_section(data, size, addr);
  }

  void init_sp(reg_t sp) { regs.write(riscv_abi_reg_t::sp, sp); }

  friend class uop_processor_t;
};

} // namespace prs
