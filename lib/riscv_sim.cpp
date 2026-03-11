#include "coregear-c/coregear.h"

#include "coregear/extensions.hpp"
#include "coregear/riscv_sim.hpp"

#include "elfio/elfio.hpp"
#include "elfio/elfio_symbols.hpp"

#include <cassert>
#include <limits>

namespace {

struct options_t final {
  cg::enabled_extensions_t extensions;
  std::string elf_path;
};

options_t parse_args(int argc, char *argv[]) {
  assert(argc == 3);
  return options_t{.extensions = cg::enabled_extensions_t(argv[1]),
                   .elf_path = argv[2]};
}

void coregear_run(options_t opts) {
  ELFIO::elfio reader;
  if (!reader.load(opts.elf_path))
    throw cg::cant_open_file_error(opts.elf_path);

  cg::riscv_simulator_t sim(opts.extensions, std::cerr);
  for (auto &&segment : reader.segments) {
    sim.load_section(segment->get_data(), segment->get_file_size(),
                     segment->get_virtual_address());
  }

  auto sym_sec = std::ranges::find_if(reader.sections, [](auto &sec) {
    return sec->get_type() == ELFIO::SHT_SYMTAB;
  });
  assert(sym_sec != reader.sections.end());
  ELFIO::symbol_section_accessor sym_table(reader, sym_sec->get());

  std::string name = "_start";
  ELFIO::Elf64_Addr value;
  ELFIO::Elf_Xword size;
  unsigned char bind;
  unsigned char type;
  ELFIO::Elf_Half section_index;
  unsigned char other;
  sym_table.get_symbol(name, value, size, bind, type, section_index, other);

  assert(value <= std::numeric_limits<cg::addr_t>::max());

  sim.init_sp(0x100000);

  cg::addr_t start = value;
  sim.exec_from(start);
}

} // namespace

int coregear_run(int argc, char *argv[]) CG_NOEXCEPT {
  options_t opts;
  try {
    opts = parse_args(argc, argv);
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    exit(EXIT_FAILURE);
  } catch (...) {
    exit(EXIT_FAILURE);
  }
  try {
    coregear_run(opts);
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    exit(EXIT_FAILURE);
  } catch (...) {
    exit(EXIT_FAILURE);
  }
  return EXIT_SUCCESS;
}

namespace cg {

class uop_processor_t {
public:
  static void add_func(riscv_simulator_t &sim,
                       std::span<const riscv_operand_t> ops) {
    sim.exec_reg_arithmetic(ops, uop::add_op);
  }

  static void sub_func(riscv_simulator_t &sim,
                       std::span<const riscv_operand_t> ops) {
    sim.exec_reg_arithmetic(ops, uop::sub_op);
  }

  static void or_func(riscv_simulator_t &sim,
                      std::span<const riscv_operand_t> ops) {
    sim.exec_reg_arithmetic(ops, uop::or_op);
  }

  static void xor_func(riscv_simulator_t &sim,
                       std::span<const riscv_operand_t> ops) {
    sim.exec_reg_arithmetic(ops, uop::xor_op);
  }

  static void and_func(riscv_simulator_t &sim,
                       std::span<const riscv_operand_t> ops) {
    sim.exec_reg_arithmetic(ops, uop::and_op);
  }

  static void srl_func(riscv_simulator_t &sim,
                       std::span<const riscv_operand_t> ops) {
    sim.exec_reg_arithmetic(ops, uop::srl_op);
  }

  static void sll_func(riscv_simulator_t &sim,
                       std::span<const riscv_operand_t> ops) {
    sim.exec_reg_arithmetic(ops, uop::sll_op);
  }

  static void sra_func(riscv_simulator_t &sim,
                       std::span<const riscv_operand_t> ops) {
    sim.exec_reg_arithmetic(ops, uop::sra_op);
  }

  static void slt_func(riscv_simulator_t &sim,
                       std::span<const riscv_operand_t> ops) {
    sim.exec_reg_arithmetic(ops, uop::slt_op);
  }

  static void sltu_func(riscv_simulator_t &sim,
                        std::span<const riscv_operand_t> ops) {
    sim.exec_reg_arithmetic(ops, uop::sltu_op);
  }

  static void addi_func(riscv_simulator_t &sim,
                        std::span<const riscv_operand_t> ops) {
    sim.exec_imm_arithmetic(ops, uop::add_op);
  }

  static void ori_func(riscv_simulator_t &sim,
                       std::span<const riscv_operand_t> ops) {
    sim.exec_imm_arithmetic(ops, uop::or_op);
  }

  static void xori_func(riscv_simulator_t &sim,
                        std::span<const riscv_operand_t> ops) {
    sim.exec_imm_arithmetic(ops, uop::xor_op);
  }

  static void andi_func(riscv_simulator_t &sim,
                        std::span<const riscv_operand_t> ops) {
    sim.exec_imm_arithmetic(ops, uop::and_op);
  }

  static void srli_func(riscv_simulator_t &sim,
                        std::span<const riscv_operand_t> ops) {
    sim.exec_imm_arithmetic(ops, uop::srl_op);
  }

  static void slli_func(riscv_simulator_t &sim,
                        std::span<const riscv_operand_t> ops) {
    sim.exec_imm_arithmetic(ops, uop::sll_op);
  }

  static void srai_func(riscv_simulator_t &sim,
                        std::span<const riscv_operand_t> ops) {
    sim.exec_imm_arithmetic(ops, uop::sra_op);
  }

  static void jalr_func(riscv_simulator_t &sim,
                        std::span<const riscv_operand_t> ops) {
    sim.exec_jalr(ops);
  }

  static void lb_func(riscv_simulator_t &sim,
                      std::span<const riscv_operand_t> ops) {
    sim.exec_load(ops, mem_op_size_t::BYTE);
  }

  static void lh_func(riscv_simulator_t &sim,
                      std::span<const riscv_operand_t> ops) {
    sim.exec_load(ops, mem_op_size_t::HALF);
  }

  static void lw_func(riscv_simulator_t &sim,
                      std::span<const riscv_operand_t> ops) {
    sim.exec_load(ops, mem_op_size_t::WORD);
  }

  static void lbu_func(riscv_simulator_t &sim,
                       std::span<const riscv_operand_t> ops) {
    sim.exec_load(ops, mem_op_size_t::UBYTE);
  }

  static void lhu_func(riscv_simulator_t &sim,
                       std::span<const riscv_operand_t> ops) {
    sim.exec_load(ops, mem_op_size_t::UHALF);
  }

  static void slti_func(riscv_simulator_t &sim,
                        std::span<const riscv_operand_t> ops) {
    sim.exec_reg_arithmetic(ops, uop::slt_op);
  }

  static void sltiu_func(riscv_simulator_t &sim,
                         std::span<const riscv_operand_t> ops) {
    sim.exec_reg_arithmetic(ops, uop::sltu_op);
  }

  static void ecall_func(riscv_simulator_t &sim,
                         std::span<const riscv_operand_t> ops) {
    sim.exec_ecall(ops);
  }

  static void ebreak_func(riscv_simulator_t &sim,
                          std::span<const riscv_operand_t> ops) {
    sim.exec_ebreak(ops);
  }

  static void fence_func(riscv_simulator_t &sim,
                         std::span<const riscv_operand_t> ops) {
    sim.exec_fence(ops);
  }

  static void jal_func(riscv_simulator_t &sim,
                       std::span<const riscv_operand_t> ops) {
    sim.exec_jal(ops);
  }

  static void beq_func(riscv_simulator_t &sim,
                       std::span<const riscv_operand_t> ops) {
    sim.exec_branch(ops, uop::eq_op);
  }

  static void bne_func(riscv_simulator_t &sim,
                       std::span<const riscv_operand_t> ops) {
    sim.exec_branch(ops, uop::ne_op);
  }

  static void blt_func(riscv_simulator_t &sim,
                       std::span<const riscv_operand_t> ops) {
    sim.exec_branch(ops, uop::lt_op);
  }

  static void bge_func(riscv_simulator_t &sim,
                       std::span<const riscv_operand_t> ops) {
    sim.exec_branch(ops, uop::ge_op);
  }

  static void bltu_func(riscv_simulator_t &sim,
                        std::span<const riscv_operand_t> ops) {
    sim.exec_branch(ops, uop::ltu_op);
  }

  static void bgeu_func(riscv_simulator_t &sim,
                        std::span<const riscv_operand_t> ops) {
    sim.exec_branch(ops, uop::geu_op);
  }

  static void sb_func(riscv_simulator_t &sim,
                      std::span<const riscv_operand_t> ops) {
    sim.exec_store(ops, mem_op_size_t::BYTE);
  }

  static void sh_func(riscv_simulator_t &sim,
                      std::span<const riscv_operand_t> ops) {
    sim.exec_store(ops, mem_op_size_t::HALF);
  }

  static void sw_func(riscv_simulator_t &sim,
                      std::span<const riscv_operand_t> ops) {
    sim.exec_store(ops, mem_op_size_t::WORD);
  }

  static void lui_func(riscv_simulator_t &sim,
                       std::span<const riscv_operand_t> ops) {
    sim.exec_lui(ops);
  }

  static void auipc_func(riscv_simulator_t &sim,
                         std::span<const riscv_operand_t> ops) {
    sim.exec_auipc(ops);
  }
};

} // namespace cg

const std::unordered_map<cg::riscv_opcode_t,
                         void (*)(cg::riscv_simulator_t &,
                                  std::span<const cg::riscv_operand_t>)>
    cg::riscv_simulator_t::i_opc_actions = {
        {cg::riscv_opcode_t::ADD, cg::uop_processor_t::add_func},
        {cg::riscv_opcode_t::SUB, cg::uop_processor_t::sub_func},
        {cg::riscv_opcode_t::OR, cg::uop_processor_t::or_func},
        {cg::riscv_opcode_t::XOR, cg::uop_processor_t::xor_func},
        {cg::riscv_opcode_t::AND, cg::uop_processor_t::and_func},
        {cg::riscv_opcode_t::SRL, cg::uop_processor_t::srl_func},
        {cg::riscv_opcode_t::SLL, cg::uop_processor_t::sll_func},
        {cg::riscv_opcode_t::SRA, cg::uop_processor_t::sra_func},
        {cg::riscv_opcode_t::SLT, cg::uop_processor_t::slt_func},
        {cg::riscv_opcode_t::SLTU, cg::uop_processor_t::sltu_func},
        {cg::riscv_opcode_t::ADDI, cg::uop_processor_t::addi_func},
        {cg::riscv_opcode_t::ORI, cg::uop_processor_t::ori_func},
        {cg::riscv_opcode_t::XORI, cg::uop_processor_t::xori_func},
        {cg::riscv_opcode_t::ANDI, cg::uop_processor_t::andi_func},
        {cg::riscv_opcode_t::SRLI, cg::uop_processor_t::srli_func},
        {cg::riscv_opcode_t::SLLI, cg::uop_processor_t::slli_func},
        {cg::riscv_opcode_t::SRAI, cg::uop_processor_t::srai_func},
        {cg::riscv_opcode_t::JALR, cg::uop_processor_t::jalr_func},
        {cg::riscv_opcode_t::LB, cg::uop_processor_t::lb_func},
        {cg::riscv_opcode_t::LH, cg::uop_processor_t::lh_func},
        {cg::riscv_opcode_t::LW, cg::uop_processor_t::lw_func},
        {cg::riscv_opcode_t::LBU, cg::uop_processor_t::lbu_func},
        {cg::riscv_opcode_t::LHU, cg::uop_processor_t::lhu_func},
        {cg::riscv_opcode_t::SLTI, cg::uop_processor_t::slti_func},
        {cg::riscv_opcode_t::SLTIU, cg::uop_processor_t::sltiu_func},
        {cg::riscv_opcode_t::ECALL, cg::uop_processor_t::ecall_func},
        {cg::riscv_opcode_t::EBREAK, cg::uop_processor_t::ebreak_func},
        {cg::riscv_opcode_t::FENCE, cg::uop_processor_t::fence_func},
        {cg::riscv_opcode_t::JAL, cg::uop_processor_t::jal_func},
        {cg::riscv_opcode_t::BEQ, cg::uop_processor_t::beq_func},
        {cg::riscv_opcode_t::BNE, cg::uop_processor_t::bne_func},
        {cg::riscv_opcode_t::BLT, cg::uop_processor_t::blt_func},
        {cg::riscv_opcode_t::BGE, cg::uop_processor_t::bge_func},
        {cg::riscv_opcode_t::BLTU, cg::uop_processor_t::bltu_func},
        {cg::riscv_opcode_t::BGEU, cg::uop_processor_t::bgeu_func},
        {cg::riscv_opcode_t::SB, cg::uop_processor_t::sb_func},
        {cg::riscv_opcode_t::SH, cg::uop_processor_t::sh_func},
        {cg::riscv_opcode_t::SW, cg::uop_processor_t::sw_func},
        {cg::riscv_opcode_t::LUI, cg::uop_processor_t::lui_func},
        {cg::riscv_opcode_t::AUIPC, cg::uop_processor_t::auipc_func},
};

const std::unordered_map<cg::riscv_opcode_t,
                         void (*)(cg::riscv_simulator_t &,
                                  std::span<const cg::riscv_operand_t>)>
    cg::riscv_simulator_t::c_opc_actions = {};

const std::unordered_map<cg::riscv_opcode_t,
                         void (*)(cg::riscv_simulator_t &,
                                  std::span<const cg::riscv_operand_t>)>
    cg::riscv_simulator_t::m_opc_actions = {};
