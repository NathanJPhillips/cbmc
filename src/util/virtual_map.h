/// \file
/// Generic virtual maps.

#ifndef CPROVER_UTIL_VIRTUAL_MAP_H
#define CPROVER_UTIL_VIRTUAL_MAP_H

#include "proxy_iterator.h"
#include <boost/range/any_range.hpp>
#include <boost/range/algorithm/find.hpp>


/// Provides a map-like interface to a backing store such as JSON files or a
/// database.
/// \tparam keyt: The type of keys of the map.
/// \tparam valuet: The type of values of the map.
/// \tparam returnt: The type of accesses to values in the map.
template<typename keyt, typename valuet, typename returnt = valuet>
class virtual_mapt
{
public:
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef keyt key_type;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef returnt mapped_type;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches mapped_type
  typedef
    typename std::conditional<
      std::is_lvalue_reference<mapped_type>::value || std::is_pointer<mapped_type>::value,
      typename make_referand_const<mapped_type>::type,
      mapped_type>::type
    const_mapped_type;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef std::pair<const key_type, valuet> value_type;
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
      key_type,
      boost::forward_traversal_tag,
      const key_type &,
      std::ptrdiff_t>
    keys_ranget;

  // Iterator types
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef proxy_iteratort<value_type, false> iterator;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef proxy_iteratort<value_type, true> const_iterator;

public:
  // The virtual destructor ensures sub-classes are disposed correctly.
  virtual ~virtual_mapt()=default;

  /// Returns the size of the map.
  /// \return The size of the map.
  virtual size_type size() const
  {
    return boost::size(keys());
  }

  /// Returns whether the map contains any mappings.
  /// \return Whether the map contains any mappings.
  virtual bool empty() const { return size()==0; }

  /// Returns all the keys in the map, which could be large.
  /// \return The set of all keys in the map.
  virtual keys_ranget keys() const=0;

  /// Returns an iterator pointing to the first element in the map.
  /// \return An iterator pointing to the first element in the map.
  virtual iterator begin()=0;

  /// Returns an iterator pointing to the first element in the map.
  /// \return An iterator pointing to the first element in the map.
  virtual const_iterator begin() const=0;

  /// Returns an iterator pointing beyond the last element in the map.
  /// \return An iterator pointing beyond the last element in the map.
  virtual iterator end()=0;

  /// Returns an iterator pointing beyond the last element in the map.
  /// \return An iterator pointing beyond the last element in the map.
  virtual const_iterator end() const=0;

  /// Find an iterator pointing to an element matching the sought key.
  /// \param key: The key to search for.
  /// \return An iterator pointing to the element that matches the key searched
  ///   for or end() if no such element is found.
  virtual iterator find(const key_type &key)=0;

  /// Find an iterator pointing to an element matching the sought key.
  /// \param key: The key to search for.
  /// \return An iterator pointing to the element that matches the key searched
  ///   for or end() if no such element is found.
  virtual const_iterator find(const key_type &key) const=0;

  /// Checks whether a given key exists in the map.
  /// \param key: The key to search for.
  /// \return True if the map contains the given key.
  virtual bool contains(const key_type &key) const
  {
    return find(key)!=end();
  }

  /// Gets the value corresponding to a given key.
  /// \param key: The key to search for.
  /// \return The value corresponding to the given key.
  virtual const_mapped_type at(const key_type &key) const=0;

  /// Gets the value corresponding to a given key.
  /// \param key: The key to search for.
  /// \return The value corresponding to the given key.
  virtual mapped_type at(const key_type &key)=0;

  /// Adds a key-value pair to the map.
  /// \param key: The key of the key-value pair to add.
  /// \param value: The value of the key-value pair to add.
  virtual void set(const key_type &key, valuet &&value)=0;


  /// Gets the value corresponding to a given key, adding it if it doesn't
  /// already exist.
  /// \param key: The key to search for.
  /// \return The value corresponding to the given key.
  mapped_type operator[](const key_type &key)
  {
    if(!contains(key))
      set(key, valuet());
    return at(key);
  }

  /// Removes a key-value pair from the map.
  /// \param key: The key of the key-value pair to remove.
  virtual void erase(const key_type &key)=0;

  /// Removes all key-value pairs from the map.
  virtual void clear()=0;

  /// Flushes changes, if any.
  virtual void flush()=0;
};

#endif
