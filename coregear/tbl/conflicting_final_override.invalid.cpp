import coregear.tbl;

namespace tbl = cg::tbl;
struct left : tbl::record_class<> {
  tbl::field<int> value = 1;
};
struct right : tbl::record_class<> {
  [[= tbl::final]] tbl::field<int> value = 2;
};
struct child : tbl::definition<left, right> {
  [[= tbl::override]] tbl::field<int> value = 3;
};
constexpr auto schema = tbl::record_schema<child>();
