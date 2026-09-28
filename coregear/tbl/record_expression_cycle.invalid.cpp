import coregear.tbl;
namespace tbl = cg::tbl;
struct second;
struct first : tbl::definition<> {
  tbl::field<int> value = tbl::record_ref<int, second, "value">;
};
struct second : tbl::definition<> {
  tbl::field<int> value = tbl::record_ref<int, first, "value">;
};
constexpr auto result = tbl::resolved_value<first, "value">();
