// -----------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2021, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2021, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file and also available at: https://github.com/seqan/raptor/blob/master/LICENSE.md
// -----------------------------------------------------------------------------------------------------

#pragma once

#include <raptor/argument_parsing/shared.hpp>

namespace raptor
{

void init_search_parser(seqan3::argument_parser & parser, search_arguments & arguments);
void run_search(seqan3::argument_parser & parser, bool const is_socks);

} // namespace raptor