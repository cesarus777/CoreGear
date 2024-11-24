#pragma once

#include "prs/exceptions.hpp"
#include "prs/logger.hpp"

#include <climits>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <iomanip>
#include <vector>

namespace prs {

using addr_t = uint32_t;

using byte_t = int8_t;
using half_t = int16_t;
using word_t = int32_t;
using ubyte_t = uint8_t;
using uhalf_t = uint16_t;
using uword_t = uint32_t;

constexpr size_t riscv_bytes_in_word = 4;
constexpr size_t riscv_bits_in_byte = 8;
constexpr size_t num_bytes_in_inst = 4;
constexpr size_t num_bits_in_inst = riscv_bits_in_byte * num_bytes_in_inst;

constexpr size_t host_byte_bits = CHAR_BIT;

static_assert(num_bits_in_inst == sizeof(word_t) * host_byte_bits);

class memory_t final {
  using cell_t = unsigned long long;

  logger_t logger;
  std::vector<cell_t> storage;

  static constexpr auto bytes_per_cell = sizeof(cell_t) / sizeof(byte_t);
  static constexpr auto halfs_per_cell = sizeof(cell_t) / sizeof(half_t);
  static constexpr auto words_per_cell = sizeof(cell_t) / sizeof(word_t);
  static constexpr auto storage_alignment = bytes_per_cell;

  static constexpr uword_t byte_mask = ~ubyte_t{0};
  static constexpr uword_t half_mask = ~uhalf_t{0};
  static constexpr uword_t word_mask = ~uword_t{0};

  static constexpr auto cell_byte_mask = static_cast<cell_t>(byte_mask);
  static constexpr auto cell_half_mask = static_cast<cell_t>(half_mask);
  static constexpr auto cell_word_mask = static_cast<cell_t>(word_mask);

  bool is_aligned_address(addr_t addr) { return addr % storage_alignment == 0; }

  void ensure_storage_capacity(addr_t addr) {
    // assert(is_aligned_address(addr));
    auto storage_idx = addr / bytes_per_cell;
    if (storage_idx < storage.size())
      return;

    storage.resize(storage_idx + 1, 0);
  }

  static constexpr size_t hex_addr_width = hex_width<addr_t>;

  enum class mem_access_t { READ, WRITE };

  auto mem_access_to_string(mem_access_t mem_access) {
    switch (mem_access) {
    default:
      return " ??? ";
    case mem_access_t::READ:
      return " ==> ";
    case mem_access_t::WRITE:
      return " <== ";
    }
  }

  template <typename data_t>
  void log_mem_access(addr_t addr, data_t data, mem_access_t access_type) {
    auto access_str = mem_access_to_string(access_type);
    logger << "[mem:0x" << std::hex << std::setw(hex_addr_width) << addr << "]"
           << access_str << "0x" << std::setw(hex_width<data_t>) << data
           << std::dec << "\n";
    logger.flush();
  }

  template <typename data_t> void log_write(addr_t addr, data_t data) {
    log_mem_access(addr, data, mem_access_t::WRITE);
  }

  template <typename data_t> void write_impl(addr_t addr, data_t data) {
    static_assert(sizeof(cell_t) >= sizeof(data_t));
    ensure_storage_capacity(addr);
    auto storage_idx = addr / bytes_per_cell;
    auto &cell = *reinterpret_cast<data_t *>(
        reinterpret_cast<ubyte_t *>(&storage[storage_idx]) +
        addr % bytes_per_cell);
    cell &= ~cell_byte_mask;
    cell |= data;
    log_write(addr, data);
  }

  template <typename data_t> void log_read(addr_t addr, data_t data) {
    log_mem_access(addr, data, mem_access_t::READ);
  }

public:
  memory_t(logger_t l) : logger(std::move(l)) {}

  ubyte_t read_byte(addr_t addr) {
    ensure_storage_capacity(addr);
    ubyte_t data =
        *(reinterpret_cast<ubyte_t *>(&storage[addr / bytes_per_cell]) +
          addr % bytes_per_cell);
    return data;
  }

  uhalf_t read_half(addr_t addr) {
    ensure_storage_capacity(addr);
    uhalf_t data = *reinterpret_cast<uhalf_t *>(
        reinterpret_cast<ubyte_t *>(&storage[addr / bytes_per_cell]) +
        addr % bytes_per_cell);
    return data;
  }

  uword_t read_word(addr_t addr) {
    ensure_storage_capacity(addr);
    uword_t data = *reinterpret_cast<uword_t *>(
        reinterpret_cast<ubyte_t *>(&storage[addr / bytes_per_cell]) +
        addr % bytes_per_cell);
    return data;
  }

  void write_byte(addr_t addr, ubyte_t data) { write_impl(addr, data); }

  void write_half(addr_t addr, uhalf_t data) { write_impl(addr, data); }

  void write_word(addr_t addr, uword_t data) { write_impl(addr, data); }

  void load_section(const char *data, size_t size, addr_t addr) {
    PRS_ASSERT((addr % bytes_per_cell) == 0);
    if (size % storage_alignment)
      size = (size / storage_alignment + 1) * storage_alignment;
    ensure_storage_capacity(addr + size);
    auto *data_end = data + size;
    while (data != data_end) {
      cell_t new_cell = 0;
      auto n = sizeof(cell_t) / sizeof(char);
      std::memcpy(&new_cell, data, n);
      storage[addr / bytes_per_cell] = new_cell;
      data += n;
      addr += n;
    }
  }

  void extend_upto(addr_t addr) { ensure_storage_capacity(addr); }
};

} // namespace prs
