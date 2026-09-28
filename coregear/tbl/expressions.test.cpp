import coregear.tbl;

namespace tbl = cg::tbl;

struct sample : tbl::definition<> {
  tbl::field<int> base = 3;
  tbl::field<int> width = tbl::ref<int, "base"> + tbl::literal(2);
  tbl::field<int> doubled = tbl::ref<int, "width"> + tbl::ref<int, "width">;
  tbl::field<int> absent;
  tbl::field<int> propagated = tbl::ref<int, "absent"> + tbl::literal(1);
  tbl::field<int> difference = tbl::ref<int, "width"> - tbl::literal(2);
  tbl::field<bool> comparison =
      tbl::equal(tbl::ref<int, "difference">, tbl::literal(3));
  tbl::field<int> selected = tbl::select(tbl::ref<bool, "comparison">,
                                         tbl::literal(12), tbl::literal(9));
  tbl::field<tbl::bits<8>> fixed = tbl::bits<8>{0xf0};
  tbl::field<tbl::bits<8>> masked = tbl::bit_and(
      tbl::ref<tbl::bits<8>, "fixed">, tbl::literal(tbl::bits<8>{0x3c}));
  tbl::field<tbl::bits<16>> combined = tbl::concat_bits(
      tbl::ref<tbl::bits<8>, "fixed">, tbl::ref<tbl::bits<8>, "masked">);
  tbl::field<std::array<int, 2>> first_list = std::array{1, 2};
  tbl::field<std::array<int, 1>> second_list = std::array{3};
  tbl::field<std::array<int, 3>> combined_list =
      tbl::concat_list(tbl::ref<std::array<int, 2>, "first_list">,
                       tbl::ref<std::array<int, 1>, "second_list">);
};

static_assert(tbl::resolved_value<sample, "width">().value == 5);
static_assert(tbl::resolved_value<sample, "doubled">().value == 10);
static_assert(!tbl::resolved_value<sample, "propagated">().is_set);
static_assert(tbl::resolved_value<sample, "selected">().value == 12);
static_assert(tbl::resolved_value<sample, "combined">().value.value == 0xf030);
static_assert(tbl::resolved_value<sample, "combined_list">().value[2] == 3);

struct external : tbl::definition<> {
  tbl::field<int> base = 7;
  tbl::field<int> absent;
};
struct inherited : tbl::record_class<> {
  tbl::field<int> inherited_value = tbl::ref<int, "local"> * tbl::literal(2);
};
struct operations : tbl::definition<inherited> {
  tbl::field<int> local = 6;
  tbl::field<int> base = tbl::record_ref<int, external, "base">;
  tbl::field<int> missing = tbl::record_ref<int, external, "absent">;
  tbl::field<int> arithmetic =
      -(tbl::literal(23) % tbl::literal(7)) * tbl::literal(6) / tbl::literal(3);
  tbl::field<bool> ne = tbl::not_equal(tbl::literal(1), tbl::literal(2));
  tbl::field<bool> lt = tbl::less(tbl::literal(1), tbl::literal(2));
  tbl::field<bool> le = tbl::less_equal(tbl::literal(2), tbl::literal(2));
  tbl::field<bool> gt = tbl::greater(tbl::literal(3), tbl::literal(2));
  tbl::field<bool> ge = tbl::greater_equal(tbl::literal(2), tbl::literal(2));
  tbl::field<bool> condition;
  tbl::field<int> unknown = tbl::select(tbl::ref<bool, "condition">,
                                        tbl::literal(1), tbl::literal(2));
  tbl::field<int> lazy = tbl::select(tbl::literal(true), tbl::literal(9),
                                     tbl::literal(1) / tbl::literal(0));
  tbl::field<int> unset_branch = tbl::select(
      tbl::literal(true), tbl::ref<int, "missing">, tbl::literal(8));
  tbl::field<int> unset_unary = -tbl::ref<int, "missing">;
  tbl::field<std::array<int, 1>> absent_list;
  tbl::field<int> unset_element =
      tbl::list_at<0>(tbl::ref<std::array<int, 1>, "absent_list">);
  tbl::field<int> selected_unset = tbl::select(
      tbl::literal(false), tbl::ref<int, "missing">, tbl::literal(8));
  tbl::field<tbl::bits<8>> bitwise =
      tbl::bit_xor(tbl::bit_or(tbl::literal(tbl::bits<8>{0xf0}),
                               tbl::literal(tbl::bits<8>{0x0c})),
                   tbl::bit_not(tbl::literal(tbl::bits<8>{0xf0})));
  tbl::field<tbl::bits<4>> slice =
      tbl::slice_bits<7, 4>(tbl::ref<tbl::bits<8>, "bitwise">);
  tbl::field<tbl::bits<64>> shifted =
      tbl::shift_left(tbl::literal(tbl::bits<64>{1}), tbl::literal(63));
  tbl::field<tbl::bits<64>> right =
      tbl::shift_right(tbl::ref<tbl::bits<64>, "shifted">, tbl::literal(63));
  tbl::field<tbl::bits<64>> unchanged =
      tbl::shift_left(tbl::ref<tbl::bits<64>, "shifted">, tbl::literal(0));
  tbl::field<tbl::bits<64>> zero =
      tbl::shift_left(tbl::ref<tbl::bits<64>, "shifted">, tbl::literal(64));
  tbl::field<tbl::bits<8>> zero_wide =
      tbl::shift_right(tbl::literal(tbl::bits<8>{255}), tbl::literal(100));
  tbl::field<std::array<int, 0>> empty = std::array<int, 0>{};
  tbl::field<std::array<int, 0>> empty_concat =
      tbl::concat_list(tbl::ref<std::array<int, 0>, "empty">,
                       tbl::ref<std::array<int, 0>, "empty">);
  tbl::field<std::size_t> size =
      tbl::list_size(tbl::ref<std::array<int, 0>, "empty_concat">);
  tbl::field<int> element = tbl::list_at<1>(tbl::concat_list(
      tbl::literal(std::array{4}), tbl::literal(std::array{5})));
};
static_assert(tbl::resolved_value<operations, "inherited_value">().value == 12);
static_assert(tbl::resolved_value<operations, "base">().value == 7);
static_assert(!tbl::resolved_value<operations, "missing">().is_set);
static_assert(tbl::resolved_value<operations, "arithmetic">().value == -4);
static_assert(tbl::resolved_value<operations, "ne">().value);
static_assert(tbl::resolved_value<operations, "lt">().value);
static_assert(tbl::resolved_value<operations, "le">().value);
static_assert(tbl::resolved_value<operations, "gt">().value);
static_assert(tbl::resolved_value<operations, "ge">().value);
static_assert(!tbl::resolved_value<operations, "unknown">().is_set);
static_assert(tbl::resolved_value<operations, "lazy">().value == 9);
static_assert(tbl::resolved_value<operations, "selected_unset">().value == 8);
static_assert(!tbl::resolved_value<operations, "unset_branch">().is_set);
static_assert(!tbl::resolved_value<operations, "unset_unary">().is_set);
static_assert(!tbl::resolved_value<operations, "unset_element">().is_set);
static_assert(tbl::resolved_value<operations, "slice">().value.value == 15);
static_assert(tbl::resolved_value<operations, "right">().value.value == 1);
static_assert(tbl::resolved_value<operations, "unchanged">().value.value ==
              (std::uint64_t{1} << 63));
static_assert(tbl::resolved_value<operations, "zero">().value.value == 0);
static_assert(tbl::resolved_value<operations, "zero_wide">().value.value == 0);
static_assert(tbl::resolved_value<operations, "size">().value == 0);
static_assert(tbl::resolved_value<operations, "element">().value == 5);
int main() { return 0; }
