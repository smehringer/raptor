// -----------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2021, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2021, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file and also available at: https://github.com/seqan/raptor/blob/master/LICENSE.md
// -----------------------------------------------------------------------------------------------------

#include <lemon/list_graph.h> /// Must be first include.

#include <raptor/build/hibf/chopper_build.hpp>
#include <raptor/build/hibf/create_ibfs_from_chopper_pack.hpp>
#include <raptor/build/store_index.hpp>
#include <raptor/index.hpp>

namespace raptor::hibf
{

void chopper_build(build_arguments const & arguments)
{
    build_data data{};
    // std::cerr << "[DEBUG] chopper_build\n"
    //           << "        &data:          " << &data << '\n'
    //           << "        &data.node_map: " << &data.node_map << "\n\n";
    create_ibfs_from_chopper_pack(data, arguments);
    // assert(false);

    std::vector<std::vector<std::string>> bin_path{};
    for (size_t i{0}; i < data.hibf.user_bins.num_user_bins(); ++i)
        bin_path.push_back(std::vector<std::string>{data.hibf.user_bins.filename_of_user_bin(i)});

    raptor_index<raptor::index_structure::hibf> index{window{arguments.window_size},
                                                      arguments.shape,
                                                      arguments.parts,
                                                      arguments.compressed,
                                                      bin_path,
                                                      std::move(data.hibf)};

    store_index(arguments.out_path, index, arguments);
}

} // namespace raptor::hibf


