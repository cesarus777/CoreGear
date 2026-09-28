import coregear.tbl;

namespace tbl = cg::tbl;

struct sample {
  [[= tbl::required]] tbl::field<int> missing;
  [[= tbl::final]] tbl::field<int> width = 32;
  [[ = tbl::override, = tbl::override_bits<7, 4> ]] tbl::field<tbl::bits<8>>
      encoding = tbl::bits<8>{0xaf};
  [[= tbl::append]] tbl::field<int> appended = 1;
  [[= tbl::prepend]] tbl::field<int> prepended = 2;
};

constexpr auto fields = tbl::direct_fields<sample>();
static_assert(std::tuple_size_v<decltype(fields)> == 5);
static_assert(std::get<0>(fields).name == "missing");
static_assert(std::get<0>(fields).source_record == ^^sample);
static_assert(std::get<0>(fields).type == ^^tbl::field<int>);
static_assert(!std::get<0>(fields).initial.is_set);
static_assert(std::get<0>(fields).annotations.required);
static_assert(std::get<1>(fields).initial.value == 32);
static_assert(std::get<1>(fields).annotations.final);
static_assert(std::get<2>(fields).initial.value.value == 0xaf);
static_assert(std::get<2>(fields).annotations.override_bits);
static_assert(std::get<2>(fields).annotations.override);
static_assert(std::get<2>(fields).annotations.high == 7);
static_assert(std::get<2>(fields).annotations.low == 4);
static_assert(std::get<3>(fields).annotations.append);
static_assert(std::get<4>(fields).annotations.prepend);

template <auto> struct structural_probe {};
using structural_field = structural_probe<tbl::field<int>{42}>;
using structural_bits = structural_probe<tbl::bits<8>{0xaf}>;

int main() { return 0; }
