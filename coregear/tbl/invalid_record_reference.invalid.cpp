import coregear.tbl;
namespace tbl = cg::tbl;
struct base : tbl::record_class<> {
  tbl::field<int> value = 1;
};
struct sample : tbl::definition<> {
  tbl::field<int> value = tbl::record_ref<int, base, "value">;
};
constexpr auto result = tbl::resolved_value<sample, "value">();
