# Columnar - Modern C++ DataFrame Library

A high-performance, type-safe C++20 library for reading and analyzing CSV data in columnar format. Designed to demonstrate modern C++ features and efficient data structures for analytical workloads.

## Why Columnar Storage?

Traditional row-based storage stores data like this:
```
[id=1, px=1.2, py=3.4, pz=5.6, energy=7.8]
[id=2, px=2.1, py=4.3, pz=6.5, energy=8.9]
[id=3, px=3.5, py=5.7, pz=7.9, energy=10.1]
```

**Columnar storage** organizes data by column:
```
id:     [1, 2, 3, ...]
px:     [1.2, 2.1, 3.5, ...]
py:     [3.4, 4.3, 5.7, ...]
pz:     [5.6, 6.5, 7.9, ...]
energy: [7.8, 8.9, 10.1, ...]
```

### Performance Benefits

1. **Better cache locality**: When analyzing one column (e.g., "energy"), all values are packed together in memory
2. **CPU-friendly**: Modern CPUs can load and process contiguous data much faster
3. **Vectorization**: SIMD instructions work better on columnar data
4. **Reduced memory traffic**: Only load the columns you need

## Features

- ✅ **Type-safe columnar storage** using `std::variant` and `std::tuple`
- ✅ **Fast CSV parsing** with `std::string_view` (zero-copy string handling)
- ✅ **Modern C++20**: concepts, ranges, custom `Expected` type
- ✅ **Filtering** with lambda predicates
- ✅ **Column access** by index (compile-time) or name (runtime)
- ✅ **Header-only library** - easy to integrate
- ✅ **Full unit tests** with Google Test
- ✅ **Comprehensive examples**
- ✅ **Compatible with Apple Clang and GCC** - no external dependencies

## Quick Start

```cpp
#include <columnar/columnar.h>

int main() {
    // Read CSV with known column types
    auto df = columnar::Columnar<int, double, double, double, double>
              ::try_read_from_csv("particles.csv");

    if (!df) {
        // Handle error
        return 1;
    }

    // Access columns by compile-time index
    auto energies = df->get_column_view<4>();

    // Access columns by name
    auto px = df->get_column_view<double>("px");

    // Filter data
    auto high_energy = df->filter<double>("energy", [](double e) {
        return e > 100.0;
    });

    std::cout << "High-energy particles: " << high_energy.num_rows() << "\n";

    return 0;
}
```

## Modern C++ Features Showcase

This project demonstrates advanced C++20 features:

### C++20 Features
- **Concepts** (`std::integral`, `std::floating_point`) - Constrain template types at compile-time
- **Ranges** (`std::views::split`, `std::views::enumerate`) - Functional-style data processing
- **std::span** - Non-owning, lightweight views of contiguous data
- **Template lambdas** - Generic lambda functions with template parameters

### Custom Implementation
- **Expected<T, E>** - Custom `std::expected`-like type for error handling (compatible with all compilers)

### C++17 Features
- **Structured bindings** - `auto [id, px, py, pz, energy] = df.get_row(i);`
- **if constexpr** - Compile-time conditional compilation
- **Fold expressions** - Compact variadic template operations
- **std::from_chars** - Fast, non-allocating number parsing
- **std::string_view** - Zero-copy string references

### Template Metaprogramming
- **Variadic templates** - `template <ColumnType... ColumnTypes>`
- **Parameter pack expansion** - Working with arbitrary numbers of types
- **std::tuple** - Type-safe heterogeneous containers
- **std::index_sequence** - Compile-time index generation

## Building

### Requirements
- C++20 compatible compiler (GCC 10+, Clang 10+, Apple Clang 12+, MSVC 2019+)
- CMake 3.20+

**Note:** This project uses a custom `Expected<T, E>` implementation instead of `std::expected` for maximum compatibility across compilers.

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/AnastasiiaHoncharenko-tech/columnar.git
cd columnar

# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build .

# Run tests
ctest --output-on-failure

# Run examples
./examples/basic_usage
./examples/filtering
./examples/particle_analysis
```

### CMake Options

```bash
cmake -DBUILD_TESTS=ON
cmake -DBUILD_EXAMPLES=ON
```

## Project Structure

```
columnar/
├── include/
│   └── columnar/
│       └── columnar.h          # Main header (header-only library)
├── tests/
│   ├── test_csv_reading.cpp    # CSV parsing tests
│   ├── test_column_access.cpp  # Column access tests
│   ├── test_filtering.cpp      # Filtering tests
│   ├── test_main.cpp           # Test runner
│   └── data/                   # Test CSV files
├── examples/
│   ├── basic_usage.cpp         # Basic operations
│   ├── filtering.cpp           # Filtering examples
│   ├── particle_analysis.cpp   # Physics analysis demo
│   └── CMakeLists.txt
├── CMakeLists.txt
├── LICENSE
└── README.md
```

## Usage Examples

### Reading CSV

```cpp
auto df = columnar::Columnar<int, double, double>::try_read_from_csv("data.csv");

if (!df) {
    switch (df.error()) {
        case columnar::CsvError::FileNotFound:
            std::cerr << "File not found\n";
            break;
        case columnar::CsvError::ParseError:
            std::cerr << "Parse error\n";
            break;
    }
    return 1;
}
```

### Accessing Data

```cpp
// By compile-time index (fastest, type-safe)
auto column0 = df->get_column_view<0>();  // First column
auto column1 = df->get_column_view<1>();  // Second column

// By name
auto energies = df->get_column_view<double>("energy");

// Get individual rows
auto [id, px, py] = df->get_row(0);
```

### Filtering

```cpp
// Single filter
auto filtered = df->filter<double>("energy", [](double e) {
    return e > 50.0;
});

// Chained filters
auto result = df->filter<double>("energy", [](double e) { return e > 50.0; })
                .filter<double>("px", [](double px) { return px > 10.0; });
```

### Statistical Analysis

```cpp
auto energies = df->get_column_view<double>("energy");

// Calculate mean
double mean = std::accumulate(energies.begin(), energies.end(), 0.0)
              / energies.size();

// Find max
double max = *std::max_element(energies.begin(), energies.end());
```

## Performance Characteristics

### Why It's Fast

1. **Zero-copy column access**: `std::span` doesn't allocate or copy data
2. **Cache-friendly filtering**: All values of a column are contiguous
3. **Compile-time type checking**: No runtime type checks needed
4. **Modern parsing**: `std::from_chars` is faster than `std::stoi` or streams

## Testing

The project includes unit tests using Google Test (24 tests):

```bash
cd build
ctest --verbose
```

Tests cover:
- CSV reading and parsing
- Column access (by index and name)
- Row access
- Filtering operations
- Error handling
- Edge cases (empty CSV files, non-existent files)
- Empty CSV with header (0 rows)
- Mixed data types

## License

MIT License - see LICENSE file for details