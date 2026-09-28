import coregear.tbl;
namespace tbl = cg::tbl;
struct other : tbl::definition<> {};
struct sample : tbl::definition<> {
  tbl::field<int> value = tbl::record_ref<int, other, "missing">;
};
constexpr auto result = tbl::resolved_value<sample, "value">();
