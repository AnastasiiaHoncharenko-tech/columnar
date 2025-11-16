#include <gtest/gtest.h>
#include "columnar/columnar.h"

using namespace columnar;

class CsvReadingTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto result = Columnar<int, int>::try_read_from_csv("data/simple.csv");
        ASSERT_TRUE(result.has_value()) << "Failed to read data/simple.csv";
        df = std::move(*result);
    }

    Columnar<int, int> df;
};

TEST_F(CsvReadingTest, ReadSimpleFile) {
    EXPECT_EQ(df.num_rows(), 5u);
    EXPECT_EQ(df.num_cols(), 2u);
}

TEST_F(CsvReadingTest, ColumnNames) {
    auto names = df.column_names();
    ASSERT_EQ(names.size(), 2u);
    EXPECT_EQ(names[0], "id");
    EXPECT_EQ(names[1], "value");
}

TEST_F(CsvReadingTest, VerifyDataValues) {
    auto ids = df.get_column_view<0>();
    auto values = df.get_column_view<1>();

    ASSERT_EQ(ids.size(), 5u);
    ASSERT_EQ(values.size(), 5u);

    EXPECT_EQ(ids[0], 1);
    EXPECT_EQ(ids[4], 5);
    EXPECT_EQ(values[0], 10);
    EXPECT_EQ(values[4], 50);
}

TEST_F(CsvReadingTest, FileNotFound) {
    auto result = Columnar<int, int>::try_read_from_csv("nonexistent.csv");

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), CsvError::FileNotFound);
}

class ParticleDataTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto result = Columnar<int, double, double, double, double>::try_read_from_csv("data/particles.csv");
        ASSERT_TRUE(result.has_value()) << "Failed to read data/particles.csv";
        df = std::move(*result);
    }

    Columnar<int, double, double, double, double> df;
};

TEST_F(ParticleDataTest, ReadParticleData) {
    EXPECT_EQ(df.num_rows(), 10u);
    EXPECT_EQ(df.num_cols(), 5u);
}

class MixedTypesTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto result = Columnar<int, double, float>::try_read_from_csv("data/mixed_types.csv");
        ASSERT_TRUE(result.has_value()) << "Failed to read data/mixed_types.csv";
        df = std::move(*result);
    }

    Columnar<int, double, float> df;
};

TEST_F(MixedTypesTest, ReadMixedTypes) {
    EXPECT_EQ(df.num_rows(), 5u);
    EXPECT_EQ(df.num_cols(), 3u);
}

class EmptyCsvTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto result = Columnar<int, int>::try_read_from_csv("data/empty_with_header.csv");
        ASSERT_TRUE(result.has_value()) << "Failed to read data/empty_with_header.csv";
        df = std::move(*result);
    }

    Columnar<int, int> df;
};

TEST_F(EmptyCsvTest, ReadEmptyWithHeader) {
    EXPECT_EQ(df.num_rows(), 0u);
    EXPECT_EQ(df.num_cols(), 2u);
}

TEST_F(EmptyCsvTest, EmptyFileHasColumnNames) {
    auto names = df.column_names();
    ASSERT_EQ(names.size(), 2u);
    EXPECT_EQ(names[0], "id");
    EXPECT_EQ(names[1], "value");
}

TEST_F(EmptyCsvTest, EmptyFileColumnViewsAreEmpty) {
    auto ids = df.get_column_view<0>();
    auto values = df.get_column_view<1>();

    EXPECT_EQ(ids.size(), 0u);
    EXPECT_EQ(values.size(), 0u);
}

TEST_F(EmptyCsvTest, FilterOnEmptyFileReturnsEmpty) {
    auto filtered_result = df.filter<int>("value", [](int v) { return v > 0; });
    ASSERT_TRUE(filtered_result.has_value());
    const auto& filtered = *filtered_result;

    EXPECT_EQ(filtered.num_rows(), 0u);
    EXPECT_EQ(filtered.num_cols(), 2u);
}

TEST(EmptyCsvNoHeaderTest, CompletelyEmptyFile) {
    auto result = Columnar<int, int>::try_read_from_csv("data/empty.csv");

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), CsvError::InvalidFormat);
}