import coregear.tbl;

struct invalid : cg::tbl::definition<> {
  cg::tbl::field<int> first = cg::tbl::ref<int, "second">;
  cg::tbl::field<int> second = cg::tbl::ref<int, "first">;
};

constexpr auto value = cg::tbl::resolved_value<invalid, "first">();
