// -----------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2021, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2021, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file and also available at: https://github.com/seqan/raptor/blob/master/LICENSE.md
// -----------------------------------------------------------------------------------------------------

#pragma once

#include <seqan3/search/views/minimiser_hash.hpp>

#include <raptor/adjust_seed.hpp>
#include <raptor/dna4_traits.hpp>
#include <raptor/search/do_parallel.hpp>
#include <raptor/search/load_index.hpp>
#include <raptor/search/precompute_threshold.hpp>
#include <raptor/search/sync_out.hpp>

namespace raptor
{

template <bool compressed>
void run_program_single_hibf(search_arguments const & arguments)
{
    using index_structure_t = std::conditional_t<compressed, index_structure::hibf_compressed, index_structure::hibf>;
    auto index = raptor_index<index_structure_t>{};

    double index_io_time{0.0};
    double reads_io_time{0.0};
    double compute_time{0.0};

    auto cereal_worker = [&] ()
    {
        load_index(index, arguments, index_io_time);
    };
    auto cereal_handle = std::async(std::launch::async, cereal_worker);

    seqan3::sequence_file_input<dna4_traits, seqan3::fields<seqan3::field::id, seqan3::field::seq>> fin{arguments.query_file};
    using record_type = typename decltype(fin)::record_type;
    std::vector<record_type> records{};

    sync_out synced_out{arguments.out_file};

    {
        size_t position{};
        std::string line{};
        for (auto const & file_list : arguments.bin_path)
        {
            line.clear();
            line = '#';
            line += std::to_string(position);
            line += '\t';
            for (auto const & filename : file_list)
            {
                line += filename;
                line += ',';
            }
            line.back() = '\n';
            synced_out << line;
            ++position;
        }
        synced_out << "#QUERY_NAME\tUSER_BINS\n";
    }

    size_t const kmers_per_window = arguments.window_size - arguments.shape_size + 1;
    size_t const kmers_per_pattern = arguments.pattern_size - arguments.shape_size + 1;
    size_t const min_number_of_minimisers = kmers_per_window == 1 ? kmers_per_pattern :
                                                std::ceil(kmers_per_pattern / static_cast<double>(kmers_per_window)); // LCOV_EXCL_LINE TODO: HIBF-BUILD
    size_t const kmer_lemma = arguments.pattern_size + 1u > (arguments.errors + 1u) * arguments.shape_size ?
                                arguments.pattern_size + 1u - (arguments.errors + 1u) * arguments.shape_size :
                                0;
    size_t const max_number_of_minimisers = arguments.pattern_size - arguments.window_size + 1;
    std::vector<size_t> const precomp_thresholds = precompute_threshold(arguments);

    auto worker = [&] (size_t const start, size_t const end)
    {
        auto & ibf = index.ibf();
        auto counter = ibf.membership_agent();
        std::string result_string{};
        std::vector<uint64_t> minimiser;

        auto hash_view = seqan3::views::minimiser_hash(arguments.shape,
                                                       seqan3::window_size{arguments.window_size},
                                                       seqan3::seed{adjust_seed(arguments.shape_weight)});

        for (auto && [id, seq] : records | seqan3::views::slice(start, end))
        {
            minimiser.clear();
            result_string.clear();
            result_string += id;
            result_string += '\t';

            minimiser = seq | hash_view | seqan3::views::to<std::vector<uint64_t>>;
            size_t const minimiser_count{minimiser.size()};

            size_t const threshold = arguments.treshold_was_set ?
                                         static_cast<size_t>(minimiser_count * arguments.threshold) :
                                         kmers_per_window == 1 ? kmer_lemma :
// LCOV_EXCL_START TODO: HIBF-BUILD
                                         precomp_thresholds[std::min(minimiser_count < min_number_of_minimisers ?
                                                                         0 :
                                                                         minimiser_count - min_number_of_minimisers,
                                                                     max_number_of_minimisers -
                                                                         min_number_of_minimisers)] + 2;
// LCOV_EXCL_STOP TODO: HIBF-BUILD
            auto & result = counter.bulk_contains(minimiser, threshold); // Results contains user bin IDs

            for (auto && count : result)
            {
                result_string += std::to_string(count);
                result_string += ',';
            }
            if (auto & last_char = result_string.back(); last_char == ',')
                last_char = '\n';
            else
                result_string += '\n';
            synced_out.write(result_string);
        }
    };

    for (auto && chunked_records : fin | seqan3::views::chunk((1ULL<<20)*10))
    {
        records.clear();
        auto start = std::chrono::high_resolution_clock::now();
        std::ranges::move(chunked_records, std::cpp20::back_inserter(records));
        auto end = std::chrono::high_resolution_clock::now();
        reads_io_time += std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

        cereal_handle.wait();

        do_parallel(worker, records.size(), arguments.threads, compute_time);
    }

// LCOV_EXCL_START
    if (arguments.write_time)
    {
        std::filesystem::path file_path{arguments.out_file};
        file_path += ".time";
        std::ofstream file_handle{file_path};
        file_handle << "Index I/O\tReads I/O\tCompute\n";
        file_handle << std::fixed
                    << std::setprecision(2)
                    << index_io_time << '\t'
                    << reads_io_time << '\t'
                    << compute_time;
    }
// LCOV_EXCL_STOP
}

} // namespace raptor
