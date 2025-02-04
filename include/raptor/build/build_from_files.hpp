// -----------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2021, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2021, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file and also available at: https://github.com/seqan/raptor/blob/master/LICENSE.md
// -----------------------------------------------------------------------------------------------------

#pragma once

#include <raptor/build/index_factory.hpp>
#include <raptor/build/store_index.hpp>

namespace raptor
{

template <bool compressed>
void build_from_files(build_arguments const & arguments)
{
    index_factory<compressed> generator{arguments};

    if (arguments.parts == 1u)
    {
        auto index = generator();
        store_index(arguments.out_path, index, arguments);
    }
    else
    {
        std::vector<std::vector<size_t>> association(arguments.parts);
        size_t next_power_of_four{4u};

        if (arguments.parts == 4u) // one-to-one
        {
            for (size_t i : std::views::iota(0u, arguments.parts))
                association[i] = std::vector<size_t>{i};
        }
        else if (arguments.parts == 2u) // More than 1 prefix per part
        {
            association[0] = std::vector<size_t>{0, 1};
            association[1] = std::vector<size_t>{2, 3};
        }
        else // More parts than prefixes
        {
            // How long must the suffix be such that 4^suffix_length >= arguments.parts
            size_t suffix_length{0};
            for (; 0b100 << (2 * suffix_length) < arguments.parts; ++suffix_length) {}
            next_power_of_four = 0b100 << (2 * suffix_length);

            size_t const prefixes_per_part = next_power_of_four / arguments.parts;

            for (size_t i : std::views::iota(0u, next_power_of_four))
                association[i/prefixes_per_part].push_back(i);
        }

        for (size_t part : std::views::iota(0u, arguments.parts))
        {
            size_t const mask{next_power_of_four - 1};
            auto filter_view = std::views::filter([&] (auto && hash)
                { return std::ranges::find(association[part], hash & mask) != association[part].end(); });

            auto index = generator(std::move(filter_view));
            std::filesystem::path out_path{arguments.out_path};
            out_path += "_" + std::to_string(part);
            store_index(out_path, index, arguments);
        }
    }
}

} // namespace raptor
