import coregear.tbl;

namespace tbl = cg::tbl;
struct base : tbl::record_class<> {
  tbl::field<int> value = 1;
};
struct child : tbl::definition<base> {
  tbl::field<int> value = 2;
};
constexpr auto schema = tbl::record_schema<child>();
