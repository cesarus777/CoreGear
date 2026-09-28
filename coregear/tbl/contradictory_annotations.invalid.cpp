import coregear.tbl;

struct invalid {
  [[ = cg::tbl::append, = cg::tbl::prepend ]] cg::tbl::field<int> value = 1;
};

constexpr auto fields = cg::tbl::direct_fields<invalid>();
