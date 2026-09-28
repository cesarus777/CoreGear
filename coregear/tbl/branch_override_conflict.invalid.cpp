import coregear.tbl;

namespace tbl = cg::tbl;
struct base : tbl::record_class<> {
  tbl::field<int> value = 1;
};
struct left : tbl::record_class<base> {
  [[= tbl::override]] tbl::field<int> value = 2;
};
struct right : tbl::record_class<base> {};
struct child : tbl::definition<left, right> {};
constexpr auto value = tbl::resolved_value<child, "value">();
