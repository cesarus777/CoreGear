import coregear.tbl;

namespace tbl = cg::tbl;
struct base : tbl::record_class<> {
  tbl::field<int> value = 1;
};
struct middle : tbl::record_class<base> {
  [[ = tbl::override, = tbl::final ]] tbl::field<int> value = 2;
};
struct child : tbl::definition<middle> {
  [[= tbl::override]] tbl::field<int> value = 3;
};
constexpr auto schema = tbl::record_schema<child>();
