import coregear.tbl;

struct invalid {
  [[= 42]] cg::tbl::field<int> value = 1;
};

constexpr auto fields = cg::tbl::direct_fields<invalid>();
