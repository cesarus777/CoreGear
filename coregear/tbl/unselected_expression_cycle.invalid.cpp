import coregear.tbl;
namespace tbl = cg::tbl;
struct sample : tbl::definition<> {
  tbl::field<int> value =
      tbl::select(tbl::literal(true), tbl::literal(1), tbl::ref<int, "value">);
};
constexpr auto result = tbl::resolved_value<sample, "value">();
