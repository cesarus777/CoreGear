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

struct defaults : tbl::record_class<> {
  [[= tbl::required]] tbl::field<int> width;
  tbl::field<int> doubled = tbl::ref<int, "width"> * tbl::literal(2);
};
struct configured : tbl::definition<defaults> {
  [[ = tbl::override, = tbl::final ]] tbl::field<int> width = 64;
};
constexpr auto configured_schema = tbl::record_schema<configured>();
static_assert(configured_schema[0].duplicate);
static_assert(configured_schema[2].source_member ==
              configured_schema[0].source_member);
static_assert(configured_schema[2].effective_member !=
              configured_schema[0].effective_member);
static_assert(configured_schema[2].annotations.required);
static_assert(configured_schema[2].annotations.final);
static_assert(tbl::resolved_value<configured, "width">().value == 64);
static_assert(tbl::resolved_value<configured, "doubled">().value == 128);

struct inherited_configured : tbl::definition<configured> {};
static_assert(tbl::resolved_value<inherited_configured, "width">().value == 64);
static_assert(tbl::record_schema<inherited_configured>()[2].annotations.final);

struct resolved_ambiguous : tbl::definition<changed_left, changed_right> {
  [[= tbl::override]] tbl::field<int> mode = 3;
};
constexpr auto resolved_schema = tbl::record_schema<resolved_ambiguous>();
static_assert(resolved_schema[0].duplicate && resolved_schema[1].duplicate);
static_assert(!resolved_schema[2].conflict);
static_assert(tbl::resolved_value<resolved_ambiguous, "mode">().value == 3);

struct changed_base : tbl::record_class<base> {
  [[= tbl::override]] tbl::field<int> width = 16;
};
struct changed_descendant : tbl::definition<changed_base> {};
static_assert(tbl::resolved_value<changed_descendant, "width">().value == 16);

struct changed_branch_left : tbl::record_class<changed_base> {};
struct changed_branch_right : tbl::record_class<changed_base> {};
struct changed_diamond
    : tbl::definition<changed_branch_left, changed_branch_right> {};
static_assert(tbl::merged_schema<changed_diamond>().size() == 1);
static_assert(tbl::resolved_value<changed_diamond, "width">().value == 16);

struct changed_again : tbl::definition<changed_base> {
  [[= tbl::override]] tbl::field<int> width = 8;
};
static_assert(tbl::resolved_value<changed_again, "width">().value == 8);
static_assert(tbl::record_schema<changed_again>()[2].source_member ==
              std::get<0>(tbl::direct_fields<base>()).source_member);

struct required_branch : tbl::record_class<> {
  [[= tbl::required]] tbl::field<int> mode;
};
struct optional_branch : tbl::record_class<> {
  tbl::field<int> mode = 1;
};
struct combined_branch : tbl::definition<required_branch, optional_branch> {
  [[= tbl::override]] tbl::field<int> mode = 4;
};
static_assert(tbl::record_schema<combined_branch>()[2].annotations.required);
static_assert(tbl::resolved_value<combined_branch, "mode">().value == 4);

int main() { return 0; }
