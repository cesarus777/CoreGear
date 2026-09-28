import coregear.tbl;
namespace tbl = cg::tbl;
struct sample : tbl::definition<> {
  tbl::field<tbl::bits<8>> value =
      tbl::shift_left(tbl::literal(tbl::bits<8>{1}), tbl::literal(-1));
};
constexpr auto result = tbl::resolved_value<sample, "value">();
