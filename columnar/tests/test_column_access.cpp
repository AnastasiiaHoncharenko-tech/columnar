#include <gtest/gtest.h>
#include "columnar/columnar.h"

using namespace columnar;

class ColumnAccessTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto result = Columnar<int, int>::try_read_from_csv("data/simple.csv");
        ASSERT_TRUE(result.has_value()) << "Failed to read data/simple.csv";
        df = std::move(*result);
    }

    Columnar<int, int> df;
};

TEST_F(ColumnAccessTest, GetColumnByIndex) {
    auto column = df.get_column_view<0>();

    ASSERT_EQ(column.size(), 5u);
    EXPECT_EQ(column[0], 1);
    EXPECT_EQ(column[1], 2);
    EXPECT_EQ(column[2], 3);
}

TEST_F(ColumnAccessTest, GetColumnByName) {
    auto result = df.get_column_view<int>("id");
    ASSERT_TRUE(result.has_value());
    auto column = *result;

    ASSERT_EQ(column.size(), 5u);
    EXPECT_EQ(column[0], 1);
    EXPECT_EQ(column[4], 5);
}

TEST_F(ColumnAccessTest, GetRow) {
    auto result = df.get_row(0);
    ASSERT_TRUE(result.has_value());
    auto row = *result;

    EXPECT_EQ(std::get<0>(row), 1);
    EXPECT_EQ(std::get<1>(row), 10);

    auto result2 = df.get_row(2);
    ASSERT_TRUE(result2.has_value());
    auto row2 = *result2;

    EXPECT_EQ(std::get<0>(row2), 3);
    EXPECT_EQ(std::get<1>(row2), 30);
}

TEST_F(ColumnAccessTest, GetRowOutOfBounds) {
    auto result = df.get_row(100);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), CsvError::RowIndexOutOfBounds);
}

TEST_F(ColumnAccessTest, ColumnNotFound) {
    auto result = df.get_column_view<int>("nonexistent");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), CsvError::ColumnNotFound);
}

TEST_F(ColumnAccessTest, ColumnWrongType) {
    auto result = df.get_column_view<double>("id");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), CsvError::ParseError);
}


TEST_F(ColumnAccessTest, SpanIsNonOwning) {
    auto column = df.get_column_view<0>();
    // NOLINTNEXTLINE(readability-container-size-empty)
    EXPECT_EQ(sizeof(column), 2 * sizeof(void*));
}