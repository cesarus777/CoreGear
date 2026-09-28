import coregear.tbl;

struct invalid : cg::tbl::definition<> {
  cg::tbl::field<unsigned> source = 1;
  cg::tbl::field<int> result = cg::tbl::ref<int, "source">;
};

constexpr auto value = cg::tbl::resolved_value<invalid, "result">();
