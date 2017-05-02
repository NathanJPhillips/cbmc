/// \file
/// virtual_mapt wrapper for std::map

#ifndef CPROVER_UTIL_MAP_WRAPPER_H
#define CPROVER_UTIL_MAP_WRAPPER_H

#include "virtual_map.h"
#include "proxy_wrapped_iterator.h"
#include <boost/range/adaptor/map.hpp>


/// Provides a virtual_mapt interface to a std::map.
/// \tparam keyt: The type of keys of the map.
/// \tparam valuet: The type of values of the map.
template<typename keyt, typename valuet>
class map_wrappert:public virtual_mapt<keyt, valuet, valuet &>
{
public:
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef typename map_wrappert::key_type key_type;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef typename map_wrappert::mapped_type mapped_type;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches mapped_type
  typedef typename map_wrappert::const_mapped_type const_mapped_type;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef typename map_wrappert::size_type size_type;

  typedef typename map_wrappert::keys_ranget keys_ranget;

  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef typename map_wrappert::iterator iterator;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef typename map_wrappert::const_iterator const_iterator;

protected:
  typedef std::map<key_type, valuet> underlying_mapt;

private:
  underlying_mapt &underlying_map;

private:
  typedef
    proxy_wrapped_iteratort<
      typename underlying_mapt::iterator,
      typename underlying_mapt::const_iterator>
    iterator_wrappert;
  typedef
    proxy_wrapped_iteratort<typename underlying_mapt::const_iterator>
    const_iterator_wrappert;

public:
  /// Creates a map_wrappert.
  /// \param underlying_map: The std::map to wrap.
  explicit map_wrappert(underlying_mapt &underlying_map)
    : underlying_map(underlying_map)
  {
  }

public:
  /// Returns the size of the map.
  /// \return The size of the map.
  virtual size_type size() const override
  {
    return underlying_map.size();
  }

  /// Returns whether the map contains any mappings.
  /// \return Whether the map contains any mappings.
  virtual bool empty() const { return underlying_map.empty(); }

  /// Returns all the keys in the map, which could be large.
  /// \return The set of all keys in the map.
  virtual keys_ranget keys() const override
  {
    return underlying_map | boost::adaptors::map_keys;
  }

  /// Returns an iterator pointing to the first element in the map.
  /// \return An iterator pointing to the first element in the map.
  virtual iterator begin() override
  {
    return iterator_wrappert(underlying_map.begin());
  }

  /// Returns an iterator pointing to the first element in the map.
  /// \return An iterator pointing to the first element in the map.
  virtual const_iterator begin() const override
  {
    return const_iterator_wrappert(underlying_map.begin());
  }

  /// Returns an iterator pointing beyond the last element in the map.
  /// \return An iterator pointing beyond the last element in the map.
  virtual iterator end() override
  {
    return iterator_wrappert(underlying_map.end());
  }

  /// Returns an iterator pointing beyond the last element in the map.
  /// \return An iterator pointing beyond the last element in the map.
  virtual const_iterator end() const override
  {
    return const_iterator_wrappert(underlying_map.end());
  }

  /// Find an iterator pointing to an element matching the sought key.
  /// \param key: The key to search for.
  /// \return An iterator pointing to the element that matches the key searched
  ///   for or end() if no such element is found.
  virtual iterator find(const key_type &key) override
  {
    return iterator_wrappert(underlying_map.find(key));
  }

  /// Find an iterator pointing to an element matching the sought key.
  /// \param key: The key to search for.
  /// \return An iterator pointing to the element that matches the key searched
  ///   for or end() if no such element is found.
  virtual const_iterator find(const key_type &key) const override
  {
    return const_iterator_wrappert(underlying_map.find(key));
  }

  /// Checks whether a given key exists in the map.
  /// \param key: The key to search for.
  /// \return True if the map contains the given key.
  virtual bool contains(const key_type &key) const override
  {
    return underlying_map.count(key) != 0;
  }

  /// Gets the value corresponding to a given key.
  /// \param key: The key to search for.
  /// \return The value corresponding to the given key.
  virtual const_mapped_type at(const key_type &key) const override
  {
    return underlying_map.at(key);
  }

  /// Gets the value corresponding to a given key.
  /// \param key: The key to search for.
  /// \return The value corresponding to the given key.
  virtual mapped_type at(const key_type &key) override
  {
    return underlying_map.at(key);
  }

  /// Adds a key-value pair to the map.
  /// \param key: The key of the key-value pair to add.
  /// \param value: The value of the key-value pair to add.
  virtual void set(const key_type &key, valuet &&value) override
  {
    underlying_map[key]=std::move(value);
  }

  /// Removes a key-value pair from the map.
  /// \param key: The key of the key-value pair to remove.
  virtual void erase(const key_type &key) override
  {
    underlying_map.erase(key);
  }

  /// Removes all key-value pairs from the map.
  virtual void clear() override
  {
    underlying_map.clear();
  }

  /// Flushes changes, if any.
  virtual void flush() override
  {
  }
};

#endif
