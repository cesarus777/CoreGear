import coregear.tbl;

struct invalid : cg::tbl::definition<> {
  cg::tbl::field<int> first = cg::tbl::ref<int, "missing">;
};

constexpr auto value = cg::tbl::resolved_value<invalid, "first">();
