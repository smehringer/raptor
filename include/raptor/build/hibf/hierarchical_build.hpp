// -----------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2021, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2021, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file and also available at: https://github.com/seqan/raptor/blob/master/LICENSE.md
// -----------------------------------------------------------------------------------------------------

#pragma once

#include <robin_hood.h>

#include <raptor/argument_parsing/build_arguments.hpp>
#include <raptor/build/hibf/build_data.hpp>

namespace raptor::hibf
{

size_t hierarchical_build(robin_hood::unordered_flat_set<size_t> & parent_kmers,
                    lemon::ListDigraph::Node const & current_node,
                    build_data & data,
                    build_arguments const & arguments,
                    bool is_root);

} // namespace raptor::hibf
