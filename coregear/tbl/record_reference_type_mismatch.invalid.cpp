import coregear.tbl;
namespace tbl = cg::tbl;
struct other : tbl::definition<> {
  tbl::field<int> value = 1;
};
struct sample : tbl::definition<> {
  tbl::field<bool> value = tbl::record_ref<bool, other, "value">;
};
constexpr auto result = tbl::resolved_value<sample, "value">();
