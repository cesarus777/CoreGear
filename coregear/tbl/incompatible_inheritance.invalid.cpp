import coregear.tbl;

struct left : cg::tbl::record_class<> {
  cg::tbl::field<int> width = 1;
};

struct right : cg::tbl::record_class<> {
  cg::tbl::field<unsigned> width = 2;
};

struct invalid : cg::tbl::definition<left, right> {};
constexpr auto schema = cg::tbl::record_schema<invalid>();
