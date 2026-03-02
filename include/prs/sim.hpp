#pragma once

#include "prs/riscv_sim.hpp"

#include <concepts>
#include <type_traits>
#include <variant>

#define PRS_EXPECT_CONCEPT_IMPL(con, type)                                     \
  static_assert(con<type>, #type " expected to satisfy " #con "!")

namespace prs {

template <typename T>
concept operand_info_impl_c =
    std::copy_constructible<T> && std::move_constructible<T>;

#define PRS_EXPECT_OPERAND_INFO_IMPL(type)                                     \
  PRS_EXPECT_CONCEPT_IMPL(operand_info_impl_c, type)

template <operand_info_impl_c... infos_t> class operand_info_t {
  std::variant<infos_t...> info;

public:
  template <typename T>
    requires(std::same_as<T, infos_t> || ...)
  operand_info_t(T op_info) : info(std::move(op_info)) {}

  auto &get() const & { return info; }

  template <typename self_t, typename... fs_t>
  auto visit(this self_t &&self, fs_t &&...funcs) {
    return std::visit(overloaded(std::forward<fs_t>(funcs)...),
                      std::forward<self_t>(self).info);
  }
};

template <typename T>
concept operand_impl_c =
    operand_info_impl_c<typename T::op_info_t> && std::copy_constructible<T> &&
    std::move_constructible<T>;

#define PRS_EXPECT_OPERAND_IMPL(type)                                          \
  PRS_EXPECT_CONCEPT_IMPL(operand_impl_c, type)

template <typename T> struct op_traits {};

template <operand_impl_c... ops_t> class operand_t final {
  std::variant<ops_t...> op;

public:
  template <typename op_info_t, typename... args_t>
    requires(std::same_as<op_info_t, typename ops_t::op_info_t> || ...)
  operand_t(op_info_t op_info, args_t &&...args)
      : op(typename op_traits<op_info_t>::op_type(
            op_info, std::forward<args_t>(args)...)) {}

  template <typename self_t, typename... fs_t>
  auto visit(this self_t &&self, fs_t &&...funcs) {
    return std::visit(overloaded(std::forward<fs_t>(funcs)...),
                      std::forward<self_t>(self).op);
  }
};

template <typename T>
concept operand_matcher_impl_c = requires(T x, typename T::inst_type inst) {
  { x.extract(inst) } -> std::same_as<typename T::extract_type>;
} && std::copy_constructible<T> && std::move_constructible<T>;

#define PRS_EXPECT_OPERAND_MATCHER_IMPL(type)                                  \
  PRS_EXPECT_CONCEPT_IMPL(operand_matcher_impl_c, type)

template <operand_matcher_impl_c... matchers_t> class operand_matcher_t {
  std::variant<matchers_t...> matcher;

public:
  template <typename T>
    requires(std::same_as<T, matchers_t> || ...)
  operand_matcher_t(T op_matcher) : matcher(std::move(op_matcher)) {}

  auto &get() const & { return matcher; }

  template <typename self_t, typename... fs_t>
  auto visit(this self_t &&self, fs_t &&...funcs) {
    return std::visit(overloaded(std::forward<fs_t>(funcs)...),
                      std::forward<self_t>(self).matcher);
  }
};

using encoding_id_t = size_t;

namespace utils {

template <typename T, template <typename...> typename U>
inline constexpr bool is_instance_of = false;

template <template <typename...> typename T, typename... Us>
inline constexpr bool is_instance_of<T<Us...>, T> = true;

} // namespace utils

template <typename T>
concept operand_info_c = utils::is_instance_of<T, operand_info_t>;

template <typename T>
concept operand_matcher_c = utils::is_instance_of<T, operand_matcher_t>;

template <typename T> struct op_desc_traits {};

template <typename impl_t> class operand_description_t {
public:
  using per_enc_t = std::unordered_map<encoding_id_t, std::vector<impl_t>>;

private:
  using op_info_t = typename op_desc_traits<impl_t>::op_info_t;
  using op_matcher_t = typename op_desc_traits<impl_t>::op_matcher_t;

  op_info_t op_info;
  op_matcher_t op_matcher;

public:
  operand_description_t(op_info_t info, op_matcher_t matcher) noexcept
      : op_info(info), op_matcher(std::move(matcher)) {}

  template <typename self_t, typename... fs_t>
  auto visit(this self_t &&self, fs_t &&...funcs) {
    auto op_info = std::forward<self_t>(self).op_info.get();
    auto op_matcher = std::forward<self_t>(self).op_matcher.get();
    return std::visit(overloaded(std::forward<fs_t>(funcs)...), op_info,
                      op_matcher);
  }
};

template <typename T>
  requires(std::is_enum_v<T>)
class opcode_wrapper_t {
  T opcode;

public:
  constexpr opcode_wrapper_t(T opc) : opcode(opc) {}

  constexpr operator T() noexcept { return opcode; }

  constexpr std::string_view to_str() const;
};

template <typename opc_t, operand_info_c op_t>
  requires(std::is_enum_v<opc_t>)
class decoded_inst_t {
  opcode_wrapper_t<opc_t> opcode;
  std::vector<op_t> operands;

public:
  decoded_inst_t(opc_t opc, std::ranges::input_range auto &&opers)
    requires(std::constructible_from<
                op_t, std::ranges::range_value_t<decltype(opers)>>)
      : opcode(opc),
        operands(std::ranges::begin(opers), std::ranges::end(opers)) {}

  auto opc() const noexcept { return opcode; }

  auto &ops() const & noexcept { return operands; }

  void print(logger_t l) const;
};

#if 0

class opcode_matcher_t final {
  inst_t filter_mask;
  inst_t match_mask;

public:
  consteval opcode_matcher_t(inst_t inst) noexcept
      : filter_mask(~static_cast<inst_t>(0)), match_mask(inst) {}

  consteval opcode_matcher_t(opcode_encoding_t opcode, funct3 f3,
                             funct7 f7) noexcept
      : filter_mask(opcode.filter() | f3.filter() | f7.filter()),
        match_mask(opcode | f3 | f7) {}

  constexpr bool match(inst_t inst) const noexcept {
    return (inst & filter_mask) == match_mask;
  }

  static const std::unordered_map<opcode_t, opcode_matcher_t> per_opc;
};

class inst_description_t {
  opcode_t opc;
  opcode_matcher_t opc_matcher;
  std::vector<operand_description_t> op_descs;

public:
  inst_description_t(opcode_t opcode)
      : opc(opcode), opc_matcher(matcher4opcode(opcode)),
        op_descs(descriptions4encoding(encoding4opcode(opcode))) {}

  auto opcode() const noexcept { return opc; }

  bool match_opc(inst_t inst) const noexcept { return opc_matcher.match(inst); }

  std::vector<operand_t> operands(inst_t inst) const {
    std::vector<operand_t> operands;
    for (const auto &op_desc : op_descs) {
      op_desc.visit(
          [inst, &operands](reg_operand_info_t reg_info,
                            reg_operand_matcher_t reg_matcher) {
            operands.push_back(
                operand_t(reg_info, reg_matcher.extract_reg(inst)));
          },
          [inst, &operands](imm_operand_info_t imm_info,
                            const imm_operand_matcher_t &imm_matcher) {
            operands.push_back(
                operand_t(imm_info, imm_matcher.extract_imm(inst)));
          },
          [inst, &operands](csr_operand_info_t csr_info,
                            csr_operand_matcher_t csr_matcher) {
            operands.push_back(
                operand_t(csr_info, csr_matcher.extract_csr(inst)));
          },
          [](reg_operand_info_t, const imm_operand_matcher_t &) {
            assert(false);
          },
          [](reg_operand_info_t, csr_operand_matcher_t) { assert(false); },
          [](imm_operand_info_t, reg_operand_matcher_t) { assert(false); },
          [](imm_operand_info_t, csr_operand_matcher_t) { assert(false); },
          [](csr_operand_info_t, reg_operand_matcher_t) { assert(false); },
          [](csr_operand_info_t, const imm_operand_matcher_t &) {
            assert(false);
          });
    }
    return operands;
  }

  static const std::vector<inst_description_t> all_descs;
};

class inst_info_t : private std::vector<inst_description_t> {
public:
  inst_info_t() : vector(opcodes.begin(), opcodes.end()) {}

  using vector::begin;
  using vector::cbegin;
  using vector::cend;
  using vector::end;
  using vector::rbegin;
  using vector::rend;

  using vector::size;
};

class simulator_t {
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

  template <bool is_dst = src> static size_t extract_reg(operand_t op) {
    size_t reg;
    op.visit(
        [&reg](reg_operand_t reg_op) {
          assert(reg_op.is_dst() == is_dst);
          reg = reg_op.get_reg();
        },
        [](auto other_ops) { assert(false); });
    return reg;
  }

  static reg_t extract_imm(operand_t op) {
    reg_t imm;
    op.visit([&imm](imm_operand_t imm_op) { imm = imm_op.imm(); },
             [](auto other_ops) { assert(false); });
    return imm;
  }

  void exec_reg_arithmetic(std::span<const operand_t> ops,
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

  void exec_imm_arithmetic(std::span<const operand_t> ops,
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

  void exec_load(std::span<const operand_t> ops, mem_op_size_t size) {
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

  void exec_store(std::span<const operand_t> ops, mem_op_size_t size) {
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

  void exec_branch(std::span<const operand_t> ops,
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

  void exec_jal(std::span<const operand_t> ops) {
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

  void exec_jalr(std::span<const operand_t> ops) {
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

  void exec_ecall(std::span<const operand_t> ops) {
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

  void exec_ebreak(std::span<const operand_t> ops) {
    assert(ops.empty());
    trap_cause = trap_cause_t::EBREAK;
  }

  void exec_fence(std::span<const operand_t>) {
    regs.set_pc(regs.get_pc() + pc_step);
  }

  void exec_lui(std::span<const operand_t> ops) {
    assert(ops.size() == 2);
    auto rd_op = ops[rd_idx];
    constexpr size_t imm_idx = 1;
    auto imm_op = ops[imm_idx];
    auto rd = extract_reg<dst>(rd_op);
    auto imm = extract_imm(imm_op);
    regs.write(rd, imm);
    regs.set_pc(regs.get_pc() + pc_step);
  }

  void exec_auipc(std::span<const operand_t> ops) {
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

  void log_decoded_inst(const decoded_inst_t &inst) { inst.print(logger); }

  const static std::unordered_map<
      opcode_t, void (*)(simulator_t &, std::span<const prs::operand_t>)>
      i_opc_actions;

  const static std::unordered_map<
      opcode_t, void (*)(simulator_t &, std::span<const prs::operand_t>)>
      c_opc_actions;

  const static std::unordered_map<
      opcode_t, void (*)(simulator_t &, std::span<const prs::operand_t>)>
      m_opc_actions;

public:
  simulator_t(enabled_extensions_t en_exts, logger_t l)
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
#endif

} // namespace prs
