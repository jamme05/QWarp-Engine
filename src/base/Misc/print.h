/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

// TODO: Fill with more general print/printf helpers

namespace qw::print
{
    typedef unsigned long long llu_t;
    typedef signed   long long lli_t;
    typedef unsigned long       lu_t;
    typedef signed   long       li_t;
    typedef unsigned int         u_t;
    typedef signed   int         i_t;
} // qw::print::

#define TO_LLU( Val ) static_cast< qw::print::llu_t >( Val )
#define TO_LLI( Val ) static_cast< qw::print::lli_t >( Val )
#define TO_LU( Val )  static_cast< qw::print::lu_t  >( Val )
#define TO_LI( Val )  static_cast< qw::print::li_t  >( Val )
#define TO_U( Val )   static_cast< qw::print::u_t   >( Val )
#define TO_I( Val )   static_cast< qw::print::i_t   >( Val )
