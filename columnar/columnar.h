#ifndef COLUMNAR_COLUMNAR_H
#define COLUMNAR_COLUMNAR_H

#include <array>
#include <concepts>
#include <cstddef>
#include <expected>
#include <fstream>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>

namespace columnar {

enum class CsvError { FileNotFound, ParseError, InvalidFormat };

// struct ParseError {
//   size_t row;
//   size_t column;
//   std::string_view value;
// };

// using ErrorType = std::variant<CsvError, ParseError>;

template <typename T>
concept ColumnTConcept = std::integral<T> || std::floating_point<T>;

template <ColumnTConcept... ColumnTypes> class Columnar {
  constexpr static size_t kColumnCount = sizeof...(ColumnTypes);

  std::tuple<std::vector<ColumnTypes>...> columns_;
  std::array<std::string, kColumnCount> names_;
  int row_count{0};

public:
  static Columnar<ColumnTypes...> read_from_csv(const std::string &filename);

  static std::expected<Columnar<ColumnTypes...>, CsvError>
  try_read_from_csv(const std::string &filename) noexcept;

  template <typename T>
  std::span<const T> get_column_view(const std::string &name) const;

  template <size_t I>
  auto get_column_view() const
      -> std::span<const std::tuple_element_t<I, std::tuple<ColumnTypes...>>>;

  template <class T> std::span<const T> get_column_view(size_t index) const;

  std::tuple<ColumnTypes...> get_row(size_t index) const;

  size_t num_rows() const noexcept;

  constexpr size_t num_cols() const noexcept { return sizeof...(ColumnTypes); }

  std::vector<std::string> column_names() const;
};

template <ColumnTConcept... ColumnTypes>
std::expected<Columnar<ColumnTypes...>, CsvError>
Columnar<ColumnTypes...>::try_read_from_csv(
    const std::string &filename) noexcept {
  std::ifstream file(filename);

  if (!file.is_open()) {
    return std::unexpected{CsvError::FileNotFound};
  }

  Columnar<ColumnTypes...> columnar;

  std::string row{};

  if (!std::getline(file, row)) {
    return std::unexpected{CsvError::InvalidFormat};
  }
  for (const auto [index, name] :
       row | std::views::split(',') | std::views::enumerate) {
    if (index == kColumnCount) {
      return std::unexpected{CsvError::InvalidFormat};
    }

    columnar.names_[index] = {name.begin(), name.end()};
  }

  while (std::getline(file, row)) {
    for (auto const [index, data] :
         row | std::views::split(',') | std::views::enumerate) {
      if (index == kColumnCount) {
        return std::unexpected{CsvError::InvalidFormat};
      }
    }

    using std::operator""sv;

    const auto fields =
        row | std::views::split(","sv) | std::ranges::to<std::vector>();
    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
      (([&] {
         using T = std::tuple_element_t<Is, std::tuple<ColumnTypes...>>;
         if constexpr (ColumnTConcept<T>) {
           return std::unexpected{CsvError::InvalidFormat};
         }

         T value{};
         const auto s = fields[Is];
         if (std::from_chars(s.data(), s.data() + s.size(), value).ec !=
             std::errc{}) {
           return std::unexpected{CsvError::ParseError};
         }
         std::get<Is>(columnar.columns_).push_back(std::move(value));
       }()),
       ...);
    }(std::make_index_sequence<kColumnCount>{});

    columnar.row_count++;
  }

  return columnar;
}

template class Columnar<int, double>;

} // namespace columnar

#endif // COLUMNAR_COLUMNAR_H
