import coregear.tbl;
namespace tbl = cg::tbl;
struct sample : tbl::definition<> {
  tbl::field<int> value = tbl::list_at<0>(tbl::literal(std::array<int, 0>{}));
};
constexpr auto result = tbl::resolved_value<sample, "value">();
