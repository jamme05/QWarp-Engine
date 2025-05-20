/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <map>
#include <unordered_map>

#include "allocator.h"

namespace qw
{
	template< class Key, class Value, class Compare = std::less< Key > >
	using map = std::map< Key, Value, Compare, allocator< std::pair< const Key, Value > > >;

	template< class Key, class Value, class Compare = std::less< Key > >
	using multimap = std::multimap< Key, Value, Compare, allocator< std::pair< const Key, Value > > >;

	template< class Key, class Value, class Hasher = std::hash< Key >, class KeyEq = std::equal_to< Key > >
	using unordered_map = std::unordered_map< Key, Value, Hasher, KeyEq, allocator< std::pair< const Key, Value > > >;

	template< class Key, class Value, class Hasher = std::hash< Key >, class KeyEq = std::equal_to< Key > >
	using unordered_multimap = std::unordered_multimap< Key, Value, Hasher, KeyEq, allocator< std::pair< const Key, Value > > >;
} // qw::