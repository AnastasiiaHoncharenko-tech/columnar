#include "columnar/columnar.h"
#include <iostream>
#include <iomanip>

namespace {
    // Column indices
    constexpr size_t kIdColumnIndex = 0;
    constexpr size_t kValueColumnIndex = 1;

    // Display limits
    constexpr size_t kMaxRowsToDisplay = 3;

    // Output formatting
    constexpr char kHorizontalLine = '-';
    constexpr char kAssignmentOperator = '=';

    // Table formatting
    constexpr int kTableWidth = 25;
}

int main() {
    std::cout << "=== Columnar Library - Basic Usage Example ===\n\n";

    std::cout << "Reading ../tests/data/simple.csv...\n";
    auto df = columnar::Columnar<int, int>::try_read_from_csv("../tests/data/simple.csv");

    if (!df) {
        std::cerr << "Error: Failed to read CSV file\n";
        return 1;
    }

    std::cout << "✓ Successfully loaded " << df->num_rows() << " rows, "
              << df->num_cols() << " columns\n\n";

    std::cout << "Column names:\n";
    auto names = df->column_names();
    for (size_t i = 0; i < names.size(); ++i) {
        std::cout << "  [" << i << "] " << names[i] << "\n";
    }
    std::cout << "\n";

    std::cout << "Accessing columns by index:\n";
    auto ids = df->get_column_view<kIdColumnIndex>();
    auto values = df->get_column_view<kValueColumnIndex>();

    std::cout << std::setw(10) << "ID" << std::setw(15) << "Value" << "\n";
    std::cout << std::string(kTableWidth, kHorizontalLine) << "\n";

    for (size_t i = 0; i < ids.size(); ++i) {
        std::cout << std::setw(10) << ids[i]
                  << std::setw(15) << values[i] << "\n";
    }
    std::cout << "\n";

    std::cout << "Accessing column by name ('value'):\n";
    auto value_column_result = df->get_column_view<int>("value");
    if (!value_column_result) {
        std::cerr << "Error: Failed to get column 'value'\n";
        return 1;
    }
    auto value_column = *value_column_result;
    for (size_t i = 0; i < value_column.size(); ++i) {
        std::cout << "  value[" << i << "] " << kAssignmentOperator << " " << value_column[i] << "\n";
    }
    std::cout << "\n";

    std::cout << "Accessing individual rows:\n";
    for (size_t i = 0; i < std::min(kMaxRowsToDisplay, df->num_rows()); ++i) {
        auto row_result = df->get_row(i);
        if (!row_result) {
            std::cerr << "Error: Failed to get row " << i << "\n";
            continue;
        }
        auto [id, value] = *row_result;
        std::cout << "  Row " << i << ": id" << kAssignmentOperator << id << ", value" << kAssignmentOperator << value << "\n";
    }

    std::cout << "\n✓ Basic usage example completed successfully!\n";
    return 0;
}