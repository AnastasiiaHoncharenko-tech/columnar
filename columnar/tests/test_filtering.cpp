#include <gtest/gtest.h>
#include "columnar/columnar.h"

using namespace columnar;

class FilteringTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto result = Columnar<int, int>::try_read_from_csv("data/simple.csv");
        ASSERT_TRUE(result.has_value());
        df = std::move(*result);
    }

    Columnar<int, int> df;
};

TEST_F(FilteringTest, FilterGreaterThan) {
    auto filtered_result = df.filter<int>("value", [](int v) { return v > 30; });
    ASSERT_TRUE(filtered_result.has_value());
    const auto& filtered = *filtered_result;

    EXPECT_EQ(filtered.num_rows(), 2u);
    EXPECT_EQ(filtered.num_cols(), 2u);

    auto values = filtered.get_column_view<1>();
    EXPECT_EQ(values[0], 40);
    EXPECT_EQ(values[1], 50);
}

TEST_F(FilteringTest, FilterNoMatches) {
    auto filtered_result = df.filter<int>("value", [](int v) { return v > 1000; });
    ASSERT_TRUE(filtered_result.has_value());
    const auto& filtered = *filtered_result;

    EXPECT_EQ(filtered.num_rows(), 0u);
    EXPECT_EQ(filtered.num_cols(), 2u);
}

TEST_F(FilteringTest, FilterAllMatch) {
    auto filtered_result = df.filter<int>("value", [](int v) { return v > 0; });
    ASSERT_TRUE(filtered_result.has_value());
    const auto& filtered = *filtered_result;

    EXPECT_EQ(filtered.num_rows(), df.num_rows());
}

TEST_F(FilteringTest, FilterEvenValues) {
    auto filtered_result = df.filter<int>("value", [](int v) { return v % 20 == 0; });
    ASSERT_TRUE(filtered_result.has_value());
    const auto& filtered = *filtered_result;

    EXPECT_EQ(filtered.num_rows(), 2u);

    auto ids = filtered.get_column_view<0>();
    EXPECT_EQ(ids[0], 2);
    EXPECT_EQ(ids[1], 4);
}

TEST_F(FilteringTest, ChainedFilters) {
    auto filtered1_result = df.filter<int>("value", [](int v) { return v >= 20; });
    ASSERT_TRUE(filtered1_result.has_value());
    const auto& filtered1 = *filtered1_result;

    auto filtered2_result = filtered1.filter<int>("value", [](int v) { return v <= 40; });
    ASSERT_TRUE(filtered2_result.has_value());
    const auto& filtered2 = *filtered2_result;

    EXPECT_EQ(filtered2.num_rows(), 3u);

    auto values = filtered2.get_column_view<1>();
    EXPECT_EQ(values[0], 20);
    EXPECT_EQ(values[1], 30);
    EXPECT_EQ(values[2], 40);
}

TEST_F(FilteringTest, FilterParticles) {
    auto particles = Columnar<int, double, double, double, double>::try_read_from_csv("data/particles.csv");
    ASSERT_TRUE(particles.has_value());

    auto high_energy_result = particles->filter<double>("energy", [](double e) { return e > 15.0; });
    ASSERT_TRUE(high_energy_result.has_value());
    const auto& high_energy = *high_energy_result;

    EXPECT_GT(high_energy.num_rows(), 0u);
    EXPECT_LT(high_energy.num_rows(), particles->num_rows());

    auto energies = high_energy.get_column_view<4>();
    for (double e : energies) {
        EXPECT_GT(e, 15.0);
    }
}