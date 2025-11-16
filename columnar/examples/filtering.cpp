#include "columnar/columnar.h"
#include <iostream>
#include <iomanip>

namespace {
    // Column indices
    constexpr size_t kEnergyColumnIndex = 4;
    constexpr size_t kPxColumnIndex = 1;

    // Output precision
    constexpr int kValuePrecision = 2;

    // Output formatting
    constexpr const char* kListSeparator = ", ";
    constexpr char kHorizontalLine = '-';

    // Filter thresholds
    constexpr double kHighEnergyThreshold = 15.0;
    constexpr double kHighPxThreshold = 5.0;
    constexpr double kChainedEnergyThreshold = 12.0;
    constexpr double kChainedPxThreshold = 6.0;

    // Table formatting
    constexpr int kFullTableWidth = 56;
    constexpr int kNarrowTableWidth = 32;
}

int main() {
    std::cout << "=== Columnar Library - Filtering Example ===\n\n";

    std::cout << "Reading particle physics data...\n";
    auto df = columnar::Columnar<int, double, double, double, double>::try_read_from_csv("../tests/data/particles.csv");

    if (!df) {
        std::cerr << "Error: Failed to read particles.csv\n";
        return 1;
    }

    std::cout << "✓ Loaded " << df->num_rows() << " particles\n\n";

    std::cout << "Original dataset:\n";
    std::cout << std::setw(8) << "ID"
              << std::setw(12) << "px"
              << std::setw(12) << "py"
              << std::setw(12) << "pz"
              << std::setw(12) << "Energy" << "\n";
    std::cout << std::string(kFullTableWidth, kHorizontalLine) << "\n";

    for (size_t i = 0; i < df->num_rows(); ++i) {
        auto row_result = df->get_row(i);
        if (!row_result) continue;
        auto [id, px, py, pz, energy] = *row_result;
        std::cout << std::setw(8) << id
                  << std::setw(12) << std::fixed << std::setprecision(kValuePrecision) << px
                  << std::setw(12) << py
                  << std::setw(12) << pz
                  << std::setw(12) << energy << "\n";
    }
    std::cout << "\n";

    std::cout << "Filter 1: Particles with energy > " << kHighEnergyThreshold << "\n";
    auto high_energy_result = df->filter<double>("energy", [](double e) {
        return e > kHighEnergyThreshold;
    });

    if (!high_energy_result) {
        std::cerr << "Error: Failed to filter by energy\n";
        return 1;
    }
    auto high_energy = *high_energy_result;

    std::cout << "  Found " << high_energy.num_rows() << " high-energy particles\n";

    auto energies = high_energy.get_column_view<kEnergyColumnIndex>();
    std::cout << "  Energies: ";
    for (size_t i = 0; i < energies.size(); ++i) {
        std::cout << energies[i];
        if (i < energies.size() - 1) std::cout << kListSeparator;
    }
    std::cout << "\n\n";

    std::cout << "Filter 2: Particles with px > " << kHighPxThreshold << "\n";
    auto high_px_result = df->filter<double>("px", [](double px) {
        return px > kHighPxThreshold;
    });

    if (!high_px_result) {
        std::cerr << "Error: Failed to filter by px\n";
        return 1;
    }
    auto high_px = *high_px_result;

    std::cout << "  Found " << high_px.num_rows() << " particles\n";

    auto px_values = high_px.get_column_view<kPxColumnIndex>();
    std::cout << "  px values: ";
    for (size_t i = 0; i < px_values.size(); ++i) {
        std::cout << px_values[i];
        if (i < px_values.size() - 1) std::cout << kListSeparator;
    }
    std::cout << "\n\n";

    std::cout << "Filter 3: Chained filters (energy > " << kChainedEnergyThreshold
              << " AND px > " << kChainedPxThreshold << ")\n";
    auto filtered_step1_result = df->filter<double>("energy", [](double e) {
        return e > kChainedEnergyThreshold;
    });

    if (!filtered_step1_result) {
        std::cerr << "Error: Failed to apply first filter\n";
        return 1;
    }
    auto filtered_step1 = *filtered_step1_result;

    auto filtered_step2_result = filtered_step1.filter<double>("px", [](double px) {
        return px > kChainedPxThreshold;
    });

    if (!filtered_step2_result) {
        std::cerr << "Error: Failed to apply second filter\n";
        return 1;
    }
    auto filtered_step2 = *filtered_step2_result;

    std::cout << "  After first filter:  " << filtered_step1.num_rows() << " particles\n";
    std::cout << "  After second filter: " << filtered_step2.num_rows() << " particles\n";

    if (filtered_step2.num_rows() > 0) {
        std::cout << "\n  Remaining particles:\n";
        std::cout << std::setw(8) << "ID"
                  << std::setw(12) << "px"
                  << std::setw(12) << "Energy" << "\n";
        std::cout << std::string(kNarrowTableWidth, kHorizontalLine) << "\n";

        for (size_t i = 0; i < filtered_step2.num_rows(); ++i) {
            auto row_result = filtered_step2.get_row(i);
            if (!row_result) continue;
            auto [id, px, py, pz, energy] = *row_result;
            std::cout << std::setw(8) << id
                      << std::setw(12) << std::fixed << std::setprecision(kValuePrecision) << px
                      << std::setw(12) << energy << "\n";
        }
    }

    std::cout << "\n✓ Filtering example completed successfully!\n";
    return 0;
}