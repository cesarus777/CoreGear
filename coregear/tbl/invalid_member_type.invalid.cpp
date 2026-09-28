import coregear.tbl;

struct invalid {
  int value = 1;
};

constexpr auto fields = cg::tbl::direct_fields<invalid>();
