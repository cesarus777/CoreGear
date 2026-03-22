#include <gtest/gtest.h>

import coregear.fsim.riscv;

namespace rv = cg::riscv;

namespace {

class rv32im : public ::testing::Test {
protected:
  rv32im() : Test(), sim(rv::enabled_extensions_t("im")) {}

  rv::simulator_t sim;
  static constexpr auto gpr_dst_op_info =
      rv::gpr_operand_info_t(/*is_dst=*/true);
  static constexpr auto gpr_src_op_info =
      rv::gpr_operand_info_t(/*is_dst=*/false);
  static constexpr auto default_ops =
      std::array{rv::operand_t(gpr_dst_op_info, rv::gpr_t::x1),
                 rv::operand_t(gpr_src_op_info, rv::gpr_t::x2),
                 rv::operand_t(gpr_src_op_info, rv::gpr_t::x3)};
};

TEST_F(rv32im, mul) {
  sim.regs.write(rv::gpr_t::x2, 3);
  sim.regs.write(rv::gpr_t::x3, 4);
  rv::uop_processor_t::mul_func(sim, default_ops);
  EXPECT_EQ(12, sim.regs.read(rv::gpr_t::x1));
}

TEST_F(rv32im, mulh) {
  sim.regs.write(rv::gpr_t::x2, -3);
  sim.regs.write(rv::gpr_t::x3, 4);
  rv::uop_processor_t::mulh_func(sim, default_ops);
  EXPECT_EQ(-1, sim.regs.read(rv::gpr_t::x1));
}

TEST_F(rv32im, mulhsu) {
  sim.regs.write(rv::gpr_t::x2, 3);
  sim.regs.write(rv::gpr_t::x3, -4);
  rv::uop_processor_t::mulhsu_func(sim, default_ops);
  EXPECT_EQ(2, sim.regs.read(rv::gpr_t::x1));
}

TEST_F(rv32im, mulhu) {
  sim.regs.write(rv::gpr_t::x2, -3);
  sim.regs.write(rv::gpr_t::x3, 4);
  rv::uop_processor_t::mulhu_func(sim, default_ops);
  EXPECT_EQ(3, sim.regs.read(rv::gpr_t::x1));
}

TEST_F(rv32im, div) {
  sim.regs.write(rv::gpr_t::x2, -12);
  sim.regs.write(rv::gpr_t::x3, 4);
  rv::uop_processor_t::div_func(sim, default_ops);
  EXPECT_EQ(-3, sim.regs.read(rv::gpr_t::x1));
}

TEST_F(rv32im, divu) {
  sim.regs.write(rv::gpr_t::x2, -12);
  sim.regs.write(rv::gpr_t::x3, 4);
  rv::uop_processor_t::divu_func(sim, default_ops);
  EXPECT_EQ(1073741821, sim.regs.read(rv::gpr_t::x1));
}

TEST_F(rv32im, rem) {
  sim.regs.write(rv::gpr_t::x2, -13);
  sim.regs.write(rv::gpr_t::x3, 4);
  rv::uop_processor_t::rem_func(sim, default_ops);
  EXPECT_EQ(-1, sim.regs.read(rv::gpr_t::x1));
}

TEST_F(rv32im, remu) {
  sim.regs.write(rv::gpr_t::x2, -13);
  sim.regs.write(rv::gpr_t::x3, 4);
  rv::uop_processor_t::remu_func(sim, default_ops);
  EXPECT_EQ(3, sim.regs.read(rv::gpr_t::x1));
}

} // namespace
