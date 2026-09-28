import coregear.tbl;
namespace tbl = cg::tbl;
struct sample : tbl::definition<> {
  tbl::field<tbl::bits<9>> value =
      tbl::slice_bits<8, 0>(tbl::literal(tbl::bits<8>{1}));
};
constexpr auto result = tbl::resolved_value<sample, "value">();
