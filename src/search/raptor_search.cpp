// -----------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2021, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2021, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file and also available at: https://github.com/seqan/raptor/blob/master/LICENSE.md
// -----------------------------------------------------------------------------------------------------

#include <raptor/search/run_program_multiple.hpp>
#include <raptor/search/run_program_single.hpp>
#include <raptor/search/run_program_single_hibf.hpp>
#include <raptor/search/run_program_single_socks.hpp>

namespace raptor
{

void raptor_search(search_arguments const & arguments)
{
    if (arguments.is_hibf)
    {
        run_program_single_hibf<false>(arguments);
    }
    else if (arguments.parts == 1)
    {
        if (arguments.is_socks)
        {
            if (arguments.compressed)
                run_program_single_socks<true>(arguments);
            else
                run_program_single_socks<false>(arguments);
        }
        else
        {
            if (arguments.compressed)
                run_program_single<true>(arguments);
            else
                run_program_single<false>(arguments);
        }
    }
    else
    {
        if (arguments.compressed)
            run_program_multiple<true>(arguments);
        else
            run_program_multiple<false>(arguments);
    }

    return;
}

} // namespace raptor
