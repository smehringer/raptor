// -----------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2021, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2021, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file and also available at: https://github.com/seqan/raptor/blob/master/LICENSE.md
// -----------------------------------------------------------------------------------------------------

#include "../cli_test.hpp"

struct search_ibf : public raptor_base, public testing::WithParamInterface<std::tuple<size_t, size_t, size_t>> {};

TEST_P(search_ibf, with_error)
{
    auto const [number_of_repeated_bins, window_size, number_of_errors] = GetParam();

    if (window_size == 23 && number_of_errors == 0)
        GTEST_SKIP() << "Needs dynamic threshold correction";

    cli_test_result const result = execute_app("raptor", "search",
                                                         "--output search.out",
                                                         "--error ", std::to_string(number_of_errors),
                                                         "--index ", ibf_path(number_of_repeated_bins, window_size),
                                                         "--query ", data("query.fq"));
    EXPECT_EQ(result.exit_code, 0);
    EXPECT_EQ(result.out, std::string{});
    EXPECT_EQ(result.err, std::string{});

    std::string const expected = string_from_file(search_result_path(number_of_repeated_bins,
                                                                     window_size,
                                                                     number_of_errors),
                                                  std::ios::binary);
    std::string const actual = string_from_file("search.out");

    EXPECT_EQ(expected, actual);
}

TEST_P(search_ibf, socks)
{
    auto const [number_of_repeated_bins, window_size, number_of_errors] = GetParam();

    if (window_size == 23 || number_of_errors != 0)
        GTEST_SKIP() << "SOCKS only supports exact kmers";

    cli_test_result const result = execute_app("raptor", "socks", "lookup-kmer",
                                                         "--output search.out",
                                                         "--index ", ibf_path(number_of_repeated_bins, window_size),
                                                         "--query ", data("query_socks.fq"));
    EXPECT_EQ(result.exit_code, 0);
    EXPECT_EQ(result.out, std::string{});
    EXPECT_EQ(result.err, std::string{});

    std::string const expected = string_from_file(search_result_path(number_of_repeated_bins,
                                                                     window_size,
                                                                     number_of_errors,
                                                                     true),
                                                  std::ios::binary);
    std::string const actual = string_from_file("search.out");

    EXPECT_EQ(expected, actual);
}

TEST_P(search_ibf, threshold)
{
    auto const [number_of_repeated_bins, window_size, number_of_errors] = GetParam();

    cli_test_result const result = execute_app("raptor", "search",
                                                         "--output search_threshold.out",
                                                         "--threshold 0.50",
                                                         "--index ", ibf_path(number_of_repeated_bins, window_size),
                                                         "--query ", data("query.fq"));
    EXPECT_EQ(result.exit_code, 0);
    EXPECT_EQ(result.out, std::string{});
    EXPECT_EQ(result.err, std::string{});

    std::string const expected = [&] ()
    {
        std::string const bin_list = [&] ()
        {
            std::string result;
            for (size_t i = 0; i < std::max<size_t>(1, number_of_repeated_bins * 4u); ++i)
            {
                result += std::to_string(i);
                result += ',';
            }
            result.pop_back();
            return result;
        }();

        std::string header{};
        std::string line{};
        std::ifstream search_result{search_result_path(number_of_repeated_bins,
                                                       window_size,
                                                       window_size == 23 ? 1 : number_of_errors)};
        while (std::getline(search_result, line) && line.substr(0, 6) != "query1")
        {
            header += line;
            header += '\n';
        }

        return header + "query1\t" + bin_list + "\nquery2\t" + bin_list + "\nquery3\t" + bin_list + '\n';
    }();

    std::string const actual = string_from_file("search_threshold.out");

    EXPECT_EQ(expected, actual);
}

TEST_P(search_ibf, no_hits)
{
    auto const [number_of_repeated_bins, window_size, number_of_errors] = GetParam();

    if (window_size == 23 && number_of_errors == 0)
        GTEST_SKIP() << "Needs dynamic threshold correction";

    cli_test_result const result = execute_app("raptor", "search",
                                                         "--output search.out",
                                                         "--error ", std::to_string(number_of_errors),
                                                         "--index ", ibf_path(number_of_repeated_bins, window_size),
                                                         "--query ", data("query_empty.fq"));
    EXPECT_EQ(result.exit_code, 0);
    EXPECT_EQ(result.out, std::string{});
    EXPECT_EQ(result.err, std::string{});

    std::string const expected = string_from_file(search_result_path(number_of_repeated_bins,
                                                                     window_size,
                                                                     number_of_errors,
                                                                     false,
                                                                     true),
                                                  std::ios::binary);
    std::string const actual = string_from_file("search.out");

    EXPECT_EQ(expected, actual);
}

INSTANTIATE_TEST_SUITE_P(
    search_ibf_suite,
    search_ibf,
    testing::Combine(testing::Values(0, 16, 32), testing::Values(19, 23), testing::Values(0, 1)),
    [] (testing::TestParamInfo<search_ibf::ParamType> const & info)
    {
        std::string name = std::to_string(std::max<int>(1, std::get<0>(info.param) * 4)) + "_bins_" +
                        std::to_string(std::get<1>(info.param)) + "_window_" +
                        std::to_string(std::get<2>(info.param)) + "_error";
        return name;
    });
