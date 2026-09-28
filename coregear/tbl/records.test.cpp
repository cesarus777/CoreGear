import coregear.tbl;

namespace tbl = cg::tbl;

struct base : tbl::record_class<> {
  tbl::field<int> width = 32;
};

struct child : tbl::definition<base> {
  tbl::field<int> lanes = 2;
};

constexpr auto child_schema = tbl::record_schema<child>();
constexpr auto child_ancestry = tbl::record_ancestry<child>();
static_assert(tbl::is_definition<child>);
static_assert(!tbl::is_definition<base>);
static_assert(child_schema.size() == 2);
static_assert(child_schema[0].name == "width");
static_assert(child_schema[1].name == "lanes");
static_assert(!child_schema[0].conflict && !child_schema[1].conflict);
static_assert(std::get<0>(child_ancestry) == ^^tbl::record_class<>);
static_assert(std::get<1>(child_ancestry) == ^^base);

template <int Width> struct sized : tbl::record_class<> {
  tbl::field<int> width = Width;
};
struct large : tbl::definition<sized<64>> {};
constexpr auto large_fields = tbl::all_fields<large>();
static_assert(std::get<0>(large_fields).initial.value == 64);

struct left : tbl::record_class<base> {};
struct right : tbl::record_class<base> {};
struct diamond : tbl::definition<left, right> {};
constexpr auto diamond_schema = tbl::record_schema<diamond>();
static_assert(diamond_schema.size() == 2);
static_assert(!diamond_schema[0].duplicate);
static_assert(diamond_schema[1].duplicate);
static_assert(!diamond_schema[0].conflict && !diamond_schema[1].conflict);
static_assert(tbl::merged_schema<diamond>().size() == 1);

struct changed_left : tbl::record_class<> {
  tbl::field<int> mode = 1;
};
struct changed_right : tbl::record_class<> {
  tbl::field<int> mode = 2;
};
struct ambiguous : tbl::definition<changed_left, changed_right> {};
constexpr auto ambiguous_schema = tbl::record_schema<ambiguous>();
static_assert(ambiguous_schema[0].conflict && ambiguous_schema[1].conflict);

int main() { return 0; }
