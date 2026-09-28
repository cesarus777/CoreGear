import coregear.tbl;

namespace tbl = cg::tbl;
struct child : tbl::definition<> {
  [[= tbl::override]] tbl::field<int> value = 2;
};
constexpr auto schema = tbl::record_schema<child>();
