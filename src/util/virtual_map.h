/*******************************************************************\

Module: virtual_map

Author: Nathan.Phillips@diffblue.com

Purpose: Generic virtual maps.

\*******************************************************************/

/// \file
/// virtual_map

#ifndef CPROVER_UTIL_VIRTUAL_MAP_H
#define CPROVER_UTIL_VIRTUAL_MAP_H

#include <boost/range/any_range.hpp>


/// Provides a map-like interface to a backing store such as JSON files or a
/// database.
/// \tparam keyt: The type of keys of the map.
/// \tparam valuet: The type of values of the map.
template<typename keyt, typename valuet>
class virtual_mapt
{
public:
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef keyt key_type;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef valuet mapped_type;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef std::pair<const keyt, valuet> value_type;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef value_type & reference;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef const value_type & const_reference;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef value_type * pointer;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef const value_type * const_pointer;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef std::size_t size_type;

  // The type of ranges of map keys
  typedef
    boost::any_range<
      keyt,
      boost::forward_traversal_tag,
      const keyt &,
      std::ptrdiff_t>
    keys_ranget;

  // The virtual destructor ensures sub-classes are disposed correctly.
  virtual ~virtual_mapt()=default;

  /// Returns the size of the map.
  /// \return The size of the map.
  virtual size_type size() const=0;

  /// Returns all the keys in the map, which could be large.
  /// \return The set of all keys in the map.
  virtual keys_ranget keys() const=0;

  /// Checks whether a given key exists in the map.
  /// \param key: The key to search for.
  /// \return True if the map contains the given key.
  virtual bool contains(const keyt &key) const=0;

  /// Gets the value corresponding to a given key.
  /// \param key: The key to search for.
  /// \return The value corresponding to the given key.
  virtual valuet at(const keyt &key) const=0;

  /// Adds a key-value pair to the map.
  /// \param key: The key of the key-value pair to add.
  /// \param value: The value of the key-value pair to add.
  virtual void set(const keyt &key, valuet &value)=0;

  /// Removes a key-value pair from the map.
  /// \param key: The key of the key-value pair to remove.
  virtual void erase(const keyt & key)=0;

  /// Removes all key-value pairs from the map.
  virtual void clear()=0;

  /// Flushes changes, if any.
  virtual void flush()=0;
};

#endif
