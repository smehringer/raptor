// -----------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2021, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2021, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file and also available at: https://github.com/seqan/raptor/blob/master/LICENSE.md
// -----------------------------------------------------------------------------------------------------

#pragma once

#include <raptor/build/store_index.hpp>
#include <raptor/shared.hpp>

namespace raptor
{

template <bool compressed>
void build_from_minimiser(build_arguments const & arguments)
{
    raptor_index<> index{arguments};

    auto worker = [&] (auto && zipped_view, auto &&)
        {
            uint64_t read_number;
            auto & ibf = index.ibf();

            for (auto && [file_names, bin_number] : zipped_view)
            {
                for (auto && file_name : file_names)
                {
                    std::ifstream infile{file_name, std::ios::binary};

                    while(infile.read(reinterpret_cast<char*>(&read_number), sizeof(read_number)))
                        ibf.emplace(read_number, seqan3::bin_index{bin_number});
                }
            }
        };

    call_parallel_on_bins(std::move(worker), arguments);

    if constexpr (compressed)
    {
        raptor_index<seqan3::data_layout::compressed> cindex{std::move(index)};
        store_index(arguments.out_path, cindex, arguments);
    }
    else
    {
        store_index(arguments.out_path, index, arguments);
    }
}

} // namespace raptor