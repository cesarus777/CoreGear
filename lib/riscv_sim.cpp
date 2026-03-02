#include "prs-c/prs.h"

#include "prs/extensions.hpp"
#include "prs/riscv_sim.hpp"

#include "elfio/elfio.hpp"
#include "elfio/elfio_symbols.hpp"

#include <cassert>
#include <limits>

namespace {

struct options_t final {
  prs::enabled_extensions_t extensions;
  std::string elf_path;
};

options_t parse_args(int argc, char *argv[]) {
  assert(argc == 3);
  return options_t{.extensions = prs::enabled_extensions_t(argv[1]),
                   .elf_path = argv[2]};
}

void run_prs(options_t opts) {
  ELFIO::elfio reader;
  if (!reader.load(opts.elf_path))
    throw prs::cant_open_file_error(opts.elf_path);

  prs::riscv_simulator_t sim(opts.extensions, std::cerr);
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

  assert(value <= std::numeric_limits<prs::addr_t>::max());

  sim.init_sp(0x100000);

  prs::addr_t start = value;
  sim.exec_from(start);
}

} // namespace

int run_prs(int argc, char *argv[]) PRS_NOEXCEPT {
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
    run_prs(opts);
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    exit(EXIT_FAILURE);
  } catch (...) {
    exit(EXIT_FAILURE);
  }
  return EXIT_SUCCESS;
}

namespace prs {

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

} // namespace prs

const std::unordered_map<prs::riscv_opcode_t,
                         void (*)(prs::riscv_simulator_t &,
                                  std::span<const prs::riscv_operand_t>)>
    prs::riscv_simulator_t::i_opc_actions = {
        {prs::riscv_opcode_t::ADD, prs::uop_processor_t::add_func},
        {prs::riscv_opcode_t::SUB, prs::uop_processor_t::sub_func},
        {prs::riscv_opcode_t::OR, prs::uop_processor_t::or_func},
        {prs::riscv_opcode_t::XOR, prs::uop_processor_t::xor_func},
        {prs::riscv_opcode_t::AND, prs::uop_processor_t::and_func},
        {prs::riscv_opcode_t::SRL, prs::uop_processor_t::srl_func},
        {prs::riscv_opcode_t::SLL, prs::uop_processor_t::sll_func},
        {prs::riscv_opcode_t::SRA, prs::uop_processor_t::sra_func},
        {prs::riscv_opcode_t::SLT, prs::uop_processor_t::slt_func},
        {prs::riscv_opcode_t::SLTU, prs::uop_processor_t::sltu_func},
        {prs::riscv_opcode_t::ADDI, prs::uop_processor_t::addi_func},
        {prs::riscv_opcode_t::ORI, prs::uop_processor_t::ori_func},
        {prs::riscv_opcode_t::XORI, prs::uop_processor_t::xori_func},
        {prs::riscv_opcode_t::ANDI, prs::uop_processor_t::andi_func},
        {prs::riscv_opcode_t::SRLI, prs::uop_processor_t::srli_func},
        {prs::riscv_opcode_t::SLLI, prs::uop_processor_t::slli_func},
        {prs::riscv_opcode_t::SRAI, prs::uop_processor_t::srai_func},
        {prs::riscv_opcode_t::JALR, prs::uop_processor_t::jalr_func},
        {prs::riscv_opcode_t::LB, prs::uop_processor_t::lb_func},
        {prs::riscv_opcode_t::LH, prs::uop_processor_t::lh_func},
        {prs::riscv_opcode_t::LW, prs::uop_processor_t::lw_func},
        {prs::riscv_opcode_t::LBU, prs::uop_processor_t::lbu_func},
        {prs::riscv_opcode_t::LHU, prs::uop_processor_t::lhu_func},
        {prs::riscv_opcode_t::SLTI, prs::uop_processor_t::slti_func},
        {prs::riscv_opcode_t::SLTIU, prs::uop_processor_t::sltiu_func},
        {prs::riscv_opcode_t::ECALL, prs::uop_processor_t::ecall_func},
        {prs::riscv_opcode_t::EBREAK, prs::uop_processor_t::ebreak_func},
        {prs::riscv_opcode_t::FENCE, prs::uop_processor_t::fence_func},
        {prs::riscv_opcode_t::JAL, prs::uop_processor_t::jal_func},
        {prs::riscv_opcode_t::BEQ, prs::uop_processor_t::beq_func},
        {prs::riscv_opcode_t::BNE, prs::uop_processor_t::bne_func},
        {prs::riscv_opcode_t::BLT, prs::uop_processor_t::blt_func},
        {prs::riscv_opcode_t::BGE, prs::uop_processor_t::bge_func},
        {prs::riscv_opcode_t::BLTU, prs::uop_processor_t::bltu_func},
        {prs::riscv_opcode_t::BGEU, prs::uop_processor_t::bgeu_func},
        {prs::riscv_opcode_t::SB, prs::uop_processor_t::sb_func},
        {prs::riscv_opcode_t::SH, prs::uop_processor_t::sh_func},
        {prs::riscv_opcode_t::SW, prs::uop_processor_t::sw_func},
        {prs::riscv_opcode_t::LUI, prs::uop_processor_t::lui_func},
        {prs::riscv_opcode_t::AUIPC, prs::uop_processor_t::auipc_func},
};

const std::unordered_map<prs::riscv_opcode_t,
                         void (*)(prs::riscv_simulator_t &,
                                  std::span<const prs::riscv_operand_t>)>
    prs::riscv_simulator_t::c_opc_actions = {};

const std::unordered_map<prs::riscv_opcode_t,
                         void (*)(prs::riscv_simulator_t &,
                                  std::span<const prs::riscv_operand_t>)>
    prs::riscv_simulator_t::m_opc_actions = {};
