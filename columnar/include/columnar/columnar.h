#ifndef COLUMNAR_COLUMNAR_H
#define COLUMNAR_COLUMNAR_H

#include <algorithm>
#include <array>
#include <charconv>
#include <concepts>
#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>
#include <cstdlib>
#include <cerrno>

namespace columnar {

enum class CsvError {
    FileNotFound,
    ParseError,
    InvalidFormat,
    ColumnNotFound,
    RowIndexOutOfBounds
};

template <typename T, typename E>
class Expected {
    std::variant<T, E> data_;

public:
    explicit Expected(const T& value) : data_(value) {}
    explicit Expected(T&& value) : data_(std::move(value)) {}
    explicit Expected(const E& error) : data_(error) {}
    explicit Expected(E&& error) : data_(std::move(error)) {}

    [[nodiscard]] bool has_value() const noexcept { return std::holds_alternative<T>(data_); }
    explicit operator bool() const noexcept { return has_value(); }

    const T& value() const& { return std::get<T>(data_); }
    T& value() & { return std::get<T>(data_); }
    T&& value() && { return std::get<T>(std::move(data_)); }

    const T* operator->() const { return &std::get<T>(data_); }
    T* operator->() { return &std::get<T>(data_); }

    const T& operator*() const& { return std::get<T>(data_); }
    T& operator*() & { return std::get<T>(data_); }
    T&& operator*() && { return std::get<T>(std::move(data_)); }

    const E& error() const& { return std::get<E>(data_); }
    E& error() & { return std::get<E>(data_); }
};

template <typename T>
concept ColumnType = std::integral<T> || std::floating_point<T> || std::same_as<T, std::string>;

template <ColumnType... ColumnTypes>
class Columnar {
    static constexpr size_t kColumnCount = sizeof...(ColumnTypes);
    static constexpr char kCsvDelimiter = ',';

    std::tuple<std::vector<ColumnTypes>...> columns_;
    std::array<std::string, kColumnCount> names_;
    size_t row_count_{0};

public:
    [[nodiscard]] static Expected<Columnar<ColumnTypes...>, CsvError>
    try_read_from_csv(const std::filesystem::path& filepath) noexcept;

    template <size_t I>
    [[nodiscard]] auto get_column_view() const
        -> std::span<const std::tuple_element_t<I, std::tuple<ColumnTypes...>>>;

    template <typename T>
    [[nodiscard]] Expected<std::span<const T>, CsvError>
    get_column_view(const std::string& name) const;

    [[nodiscard]] Expected<std::tuple<ColumnTypes...>, CsvError>
    get_row(size_t index) const;

    template <typename T>
    [[nodiscard]] Expected<Columnar<ColumnTypes...>, CsvError>
    filter(const std::string& column_name, std::function<bool(T)> predicate) const;

    [[nodiscard]] size_t num_rows() const noexcept { return row_count_; }

    [[nodiscard]] constexpr size_t num_cols() const noexcept { return kColumnCount; }

    [[nodiscard]] std::span<const std::string, kColumnCount>
    column_names() const noexcept;

private:
    template <typename T>
    requires(std::integral<T>)
    static Expected<T, CsvError> parse_value(std::string_view str) noexcept {
        T value{};
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);

        if (ec != std::errc{} || ptr != str.data() + str.size()) {
            return Expected<T, CsvError>(CsvError::ParseError);
        }
        return Expected<T, CsvError>(value);
    }

    template <typename T>
    requires(std::floating_point<T>)
    static Expected<T, CsvError> parse_value(std::string_view str) noexcept {
        T value{};
        std::string str_copy(str);
        char* end = nullptr;

        errno = 0;

        if constexpr (std::is_same_v<T, double>) {
            value = std::strtod(str_copy.c_str(), &end);
        } else if constexpr (std::is_same_v<T, float>) {
            value = std::strtof(str_copy.c_str(), &end);
        } else {
            value = std::strtold(str_copy.c_str(), &end);
        }

        if (end == str_copy.c_str() || *end != '\0') {
            return Expected<T, CsvError>(CsvError::ParseError);
        }
        if (errno == ERANGE) {
            return Expected<T, CsvError>(CsvError::ParseError);
        }

        return Expected<T, CsvError>(value);
    }

    template <typename T>
    requires(std::same_as<T, std::string>)
    static Expected<T, CsvError> parse_value(std::string_view str) noexcept {
        return Expected<T, CsvError>(std::string(str));
    }
};

template <ColumnType... ColumnTypes>
[[nodiscard]] Expected<Columnar<ColumnTypes...>, CsvError>
Columnar<ColumnTypes...>::try_read_from_csv(const std::filesystem::path& filepath) noexcept {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return Expected<Columnar<ColumnTypes...>, CsvError>(CsvError::FileNotFound);
    }

    Columnar<ColumnTypes...> result;
    std::string line;

    if (!std::getline(file, line) || line.empty()) {
        return Expected<Columnar<ColumnTypes...>, CsvError>(CsvError::InvalidFormat);
    }

    {
        std::istringstream header_stream(line);
        std::string name;
        size_t col_idx = 0;

        while (std::getline(header_stream, name, kCsvDelimiter)) {
            if (col_idx >= kColumnCount) {
                return Expected<Columnar<ColumnTypes...>, CsvError>(CsvError::InvalidFormat);
            }
            result.names_[col_idx++] = std::move(name);
        }

        if (col_idx != kColumnCount) {
            return Expected<Columnar<ColumnTypes...>, CsvError>(CsvError::InvalidFormat);
        }
    }

    while (std::getline(file, line)) {
        if(line.empty()) continue;

        std::istringstream row_stream(line);
        std::string value_str;

        bool parse_success = [&]<size_t... Is>(std::index_sequence<Is...>) -> bool {
            bool success = true;

            ((success = success && [&]() -> bool {
                using T = std::tuple_element_t<Is, std::tuple<ColumnTypes...>>;

                if (!std::getline(row_stream, value_str, kCsvDelimiter)) {
                    return false;
                }

                auto parsed = parse_value<T>(value_str);
                if (!parsed) {
                    return false;
                }

                std::get<Is>(result.columns_).push_back(std::move(*parsed));
                return true;
            }()), ...);

            if (row_stream >> value_str) {
                 return false;
            }

            return success;
        }(std::make_index_sequence<kColumnCount>{});

        if (!parse_success) {
            return Expected<Columnar<ColumnTypes...>, CsvError>(CsvError::ParseError);
        }

        ++result.row_count_;
    }

    return Expected<Columnar<ColumnTypes...>, CsvError>(result);
}

template <ColumnType... ColumnTypes>
template <size_t I>
[[nodiscard]] auto Columnar<ColumnTypes...>::get_column_view() const
    -> std::span<const std::tuple_element_t<I, std::tuple<ColumnTypes...>>> {
    static_assert(I < kColumnCount, "Column index out of bounds");
    const auto& column = std::get<I>(columns_);
    return std::span{column};
}

template <ColumnType... ColumnTypes>
template <typename T>
[[nodiscard]] Expected<std::span<const T>, CsvError>
Columnar<ColumnTypes...>::get_column_view(const std::string& name) const {
    auto it = std::find(names_.begin(), names_.end(), name);

    if (it == names_.end()) {
        return Expected<std::span<const T>, CsvError>(CsvError::ColumnNotFound);
    }

    size_t index = std::distance(names_.begin(), it);

    Expected<std::span<const T>, CsvError> result(CsvError::ColumnNotFound);

    [&]<size_t... Is>(std::index_sequence<Is...>) {
        (void)((Is == index ? [&] {
            using ColumnT = std::tuple_element_t<Is, std::tuple<ColumnTypes...>>;
            if constexpr (std::is_same_v<T, ColumnT>) {
                result = Expected<std::span<const T>, CsvError>(get_column_view<Is>());
            }
        }(), true : false) || ...);
    }(std::make_index_sequence<kColumnCount>{});

    if(!result.has_value() && result.error() == CsvError::ColumnNotFound) {
         return Expected<std::span<const T>, CsvError>(CsvError::ParseError);
    }

    return result;
}

template <ColumnType... ColumnTypes>
[[nodiscard]] Expected<std::tuple<ColumnTypes...>, CsvError>
Columnar<ColumnTypes...>::get_row(size_t index) const {
    if (index >= row_count_) {
        return Expected<std::tuple<ColumnTypes...>, CsvError>(CsvError::RowIndexOutOfBounds);
    }

    auto row_tuple = [&]<size_t... Is>(std::index_sequence<Is...>) {
        return std::make_tuple(std::get<Is>(columns_)[index]...);
    }(std::make_index_sequence<kColumnCount>{});

    return Expected<std::tuple<ColumnTypes...>, CsvError>(row_tuple);
}

template <ColumnType... ColumnTypes>
template <typename T>
[[nodiscard]] Expected<Columnar<ColumnTypes...>, CsvError>
Columnar<ColumnTypes...>::filter(const std::string& column_name,
                                  std::function<bool(T)> predicate) const {
    auto column_view_result = get_column_view<T>(column_name);
    if (!column_view_result) {
        return Expected<Columnar<ColumnTypes...>, CsvError>(column_view_result.error());
    }
    auto column_to_filter = *column_view_result;

    std::vector<size_t> indices_to_keep;
    indices_to_keep.reserve(row_count_);

    for (size_t i = 0; i < row_count_; ++i) {
        if (predicate(column_to_filter[i])) {
            indices_to_keep.push_back(i);
        }
    }

    Columnar<ColumnTypes...> result;
    result.names_ = names_;
    result.row_count_ = indices_to_keep.size();

    [&]<size_t... Is>(std::index_sequence<Is...>) {
        (([&] {
            auto& src_column = std::get<Is>(columns_);
            auto& dst_column = std::get<Is>(result.columns_);
            dst_column.reserve(indices_to_keep.size());

            for (size_t idx : indices_to_keep) {
                dst_column.push_back(src_column[idx]);
            }
        }()), ...);
    }(std::make_index_sequence<kColumnCount>{});

    return Expected<Columnar<ColumnTypes...>, CsvError>(result);
}

template <ColumnType... ColumnTypes>
[[nodiscard]] std::span<const std::string, Columnar<ColumnTypes...>::kColumnCount>
Columnar<ColumnTypes...>::column_names() const noexcept {
    return std::span{names_};
}

}

#endif