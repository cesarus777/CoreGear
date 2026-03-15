module;

#include <coregear/utility/assert.hpp>

#include <elfio/elfio.hpp>
#include <elfio/elfio_symbols.hpp>

#include <unistd.h>

module coregear.fsim.riscv;

void cg::riscv::simulator_t::process_write() {
  auto guest_fd = regs.read(gpr_abi_t::a0);
  auto host_fd = open_fds.at(guest_fd);
  auto addr = to_addr(regs.read(gpr_abi_t::a1));
  auto len = regs.read(gpr_abi_t::a2);
  auto written = 0;
  for (auto i : std::views::iota(0, len)) {
    char data = memory.read_byte(addr + i);
    written += write(host_fd, &data, 1);
  }
  regs.write(gpr_abi_t::a0, written);
}

void cg::riscv::coregear_run(cg::riscv::options_t opts) {
  ELFIO::elfio reader;
  if (!reader.load(opts.elf_path))
    throw cant_open_file_error(opts.elf_path);

  simulator_t sim(opts.extensions, std::cerr);
  for (auto &&segment : reader.segments) {
    sim.load_section(segment->get_data(), segment->get_file_size(),
                     segment->get_virtual_address());
  }

  auto sym_sec = std::ranges::find_if(reader.sections, [](auto &sec) {
    return sec->get_type() == ELFIO::SHT_SYMTAB;
  });
  CG_ASSERT(sym_sec != reader.sections.end());
  ELFIO::symbol_section_accessor sym_table(reader, sym_sec->get());

  std::string name = "_start";
  ELFIO::Elf64_Addr value;
  ELFIO::Elf_Xword size;
  unsigned char bind;
  unsigned char type;
  ELFIO::Elf_Half section_index;
  unsigned char other;
  sym_table.get_symbol(name, value, size, bind, type, section_index, other);

  CG_ASSERT(value <= std::numeric_limits<addr_t>::max());

  sim.init_sp(0x100000);

  addr_t start = value;
  sim.exec_from(start);
}

#ifdef __cplusplus
extern "C" {
#endif
int coregear_run(int argc, char *argv[])
#ifdef __cplusplus
    noexcept
#endif
{
  cg::riscv::options_t opts;
  try {
    opts = cg::riscv::parse_args(argc, argv);
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    std::exit(EXIT_FAILURE);
  } catch (...) {
    std::exit(EXIT_FAILURE);
  }
  try {
    cg::riscv::coregear_run(opts);
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    std::exit(EXIT_FAILURE);
  } catch (...) {
    std::exit(EXIT_FAILURE);
  }
  return EXIT_SUCCESS;
}
#ifdef __cplusplus
}
#endif
