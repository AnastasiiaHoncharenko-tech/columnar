#include "columnar/columnar.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <numeric>

namespace {
    // Column indices
    constexpr size_t kIdColumnIndex = 0;
    constexpr size_t kPxColumnIndex = 1;
    constexpr size_t kPyColumnIndex = 2;
    constexpr size_t kPzColumnIndex = 3;
    constexpr size_t kEnergyColumnIndex = 4;

    // Output precision
    constexpr int kEnergyPrecision = 3;
    constexpr int kPercentagePrecision = 1;
    constexpr int kValuePrecision = 2;

    // Table separators
    constexpr char kHorizontalLine = '-';
    constexpr char kDoubleLine = '=';

    // Selection criteria
    constexpr double kForwardPzThreshold = 10.0;
    constexpr double kPercentageMultiplier = 100.0;

    // Table formatting
    constexpr int kTableWidth = 60;
    constexpr int kNarrowTableWidth = 32;
}

template <typename T>
double calculate_mean(std::span<const T> data) {
    if (data.empty()) return 0.0;
    return std::accumulate(data.begin(), data.end(), 0.0) / data.size();
}

template <typename T>
double calculate_stddev(std::span<const T> data, double mean) {
    if (data.empty()) return 0.0;

    double sum_sq_diff = 0.0;
    for (const auto& value : data) {
        double diff = value - mean;
        sum_sq_diff += diff * diff;
    }

    return std::sqrt(sum_sq_diff / data.size());
}

int main() {
    std::cout << "=== Particle Physics Analysis with Columnar ===\n\n";
    std::cout << "This example demonstrates how columnar storage is beneficial\n";
    std::cout << "for analytical operations on particle physics data.\n\n";

    auto df = columnar::Columnar<int, double, double, double, double>::try_read_from_csv("../tests/data/particles.csv");

    if (!df) {
        std::cerr << "Error: Failed to read particles.csv\n";
        return 1;
    }

    std::cout << "Loaded " << df->num_rows() << " particle events\n";
    std::cout << std::string(kTableWidth, kDoubleLine) << "\n\n";

    std::cout << "Energy Analysis:\n";
    std::cout << std::string(kTableWidth, kHorizontalLine) << "\n";

    auto energies = df->get_column_view<kEnergyColumnIndex>();
    double energy_mean = calculate_mean(energies);
    double energy_stddev = calculate_stddev(energies, energy_mean);

    std::cout << std::fixed << std::setprecision(kEnergyPrecision);
    std::cout << "  Mean energy:      " << energy_mean << " GeV\n";
    std::cout << "  Std deviation:    " << energy_stddev << " GeV\n";
    std::cout << "  Min energy:       " << *std::min_element(energies.begin(), energies.end()) << " GeV\n";
    std::cout << "  Max energy:       " << *std::max_element(energies.begin(), energies.end()) << " GeV\n";
    std::cout << "\n";

    std::cout << "Momentum Analysis:\n";
    std::cout << std::string(kTableWidth, kHorizontalLine) << "\n";

    auto px = df->get_column_view<kPxColumnIndex>();
    auto py = df->get_column_view<kPyColumnIndex>();
    auto pz = df->get_column_view<kPzColumnIndex>();

    std::cout << std::setw(8) << "ID"
              << std::setw(12) << "px"
              << std::setw(12) << "py"
              << std::setw(12) << "pz"
              << std::setw(14) << "|p| (total)" << "\n";
    std::cout << std::string(kTableWidth, kHorizontalLine) << "\n";

    for (size_t i = 0; i < df->num_rows(); ++i) {
        double total_momentum = std::sqrt(px[i]*px[i] + py[i]*py[i] + pz[i]*pz[i]);
        auto row_result = df->get_row(i);
        if (!row_result) continue;
        auto [id, px_val, py_val, pz_val, e] = *row_result;

        std::cout << std::setw(8) << id
                  << std::setw(12) << px_val
                  << std::setw(12) << py_val
                  << std::setw(12) << pz_val
                  << std::setw(14) << total_momentum << "\n";
    }
    std::cout << "\n";

    std::cout << "Event Selection: High-energy, forward-going particles\n";
    std::cout << std::string(kTableWidth, kHorizontalLine) << "\n";
    std::cout << "Criteria: energy > " << energy_mean << " AND pz > " << kForwardPzThreshold << "\n\n";

    auto high_energy_result = df->filter<double>("energy", [energy_mean](double e) {
        return e > energy_mean;
    });

    if (!high_energy_result) {
        std::cerr << "Error: Failed to filter by energy\n";
        return 1;
    }
    auto high_energy = *high_energy_result;

    auto selected_result = high_energy.filter<double>("pz", [](double pz_val) {
        return pz_val > kForwardPzThreshold;
    });

    if (!selected_result) {
        std::cerr << "Error: Failed to filter by pz\n";
        return 1;
    }
    auto selected = *selected_result;

    std::cout << "  Events passing selection: " << selected.num_rows()
              << " / " << df->num_rows()
              << " (" << std::setprecision(kPercentagePrecision)
              << (kPercentageMultiplier * selected.num_rows() / df->num_rows()) << "%)\n\n";

    if (selected.num_rows() > 0) {
        std::cout << "Selected events:\n";
        std::cout << std::setw(8) << "ID"
                  << std::setw(12) << "Energy"
                  << std::setw(12) << "pz" << "\n";
        std::cout << std::string(kNarrowTableWidth, kHorizontalLine) << "\n";

        auto sel_energies = selected.get_column_view<kEnergyColumnIndex>();
        auto sel_pz = selected.get_column_view<kPzColumnIndex>();
        auto sel_ids = selected.get_column_view<kIdColumnIndex>();

        for (size_t i = 0; i < selected.num_rows(); ++i) {
            std::cout << std::setw(8) << sel_ids[i]
                      << std::setw(12) << std::setprecision(kValuePrecision) << sel_energies[i]
                      << std::setw(12) << sel_pz[i] << "\n";
        }
    }

    std::cout << "\n" << std::string(kTableWidth, kDoubleLine) << "\n";
    std::cout << "Why columnar storage is faster:\n";
    std::cout << "  - All energy values are packed together in memory\n";
    std::cout << "  - CPU cache can load many values at once\n";
    std::cout << "  - No need to skip over other columns (px, py, pz)\n";
    std::cout << "  - SIMD vectorization is easier for compilers\n";
    std::cout << "\n✓ Analysis completed successfully!\n";

    return 0;
}