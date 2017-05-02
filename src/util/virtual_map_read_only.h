/// \file
/// virtual_mapt implementation that hides modification methods.

#ifndef CPROVER_UTIL_VIRTUAL_MAP_READ_ONLY_H
#define CPROVER_UTIL_VIRTUAL_MAP_READ_ONLY_H

#include "virtual_map.h"


/// Provides a virtual_mapt implementation that hides modification methods.
/// \tparam keyt: The type of keys of the map.
/// \tparam valuet: The type of values of the map.
/// \tparam returnt: The type of accesses to values in the map.
template<typename keyt, typename valuet, typename returnt = valuet>
class virtual_map_read_onlyt:public virtual_mapt<keyt, valuet, returnt>
{
public:
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef typename virtual_map_read_onlyt::key_type key_type;

private:
  /// This override is not used.
  /// \param key: The key of the key-value pair to add.
  /// \param value: The value of the key-value pair to add.
  virtual void set(const key_type &key, valuet &&body) final override
  {
    throw std::logic_error("Tried to add to a virtual_map_read_onlyt");
  }

  /// This override is not used.
  /// \param key: The key of the key-value pair to remove.
  virtual void erase(const key_type &key) final override
  {
    throw std::logic_error(
      "Tried to erase an entry from a virtual_map_read_onlyt");
  }

  /// This override is not used.
  virtual void clear() final override
  {
    throw std::logic_error("Tried to clear a virtual_map_read_onlyt");
  }

  /// This override is not used.
  virtual void flush() final override
  {
    throw std::logic_error("Tried to flush a virtual_map_read_onlyt");
  }
};

#endif
