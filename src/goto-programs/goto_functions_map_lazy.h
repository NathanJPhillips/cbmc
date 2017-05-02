/// \file
/// A lazy virtual_mapt wrapper for goto_functionst.

#ifndef CPROVER_GOTO_PROGRAMS_GOTO_FUNCTIONS_MAP_LAZY_H
#define CPROVER_GOTO_PROGRAMS_GOTO_FUNCTIONS_MAP_LAZY_H

#include <util/virtual_map_read_only.h>
#include "goto_functions.h"
#include "goto_convert_functions.h"
#include <util/message.h>
#include <util/language_file.h>
#include <boost/range/adaptor/map.hpp>


/// Provides a lazy virtual_mapt wrapper for goto_functionst.
/// This incrementally builds a goto-functions object, while permitting
/// access to goto programs while they are still under construction.
/// The intended workflow:
/// 1. The frontend program registers the functions that are potentially
///   available, probably by use of util/language_files.h
/// 2. The frontend registers functions that should be run on
///   each program, in sequence, after it is converted.
/// 3. Analyses access functions using the at function
template<typename bodyt>
class goto_functions_map_lazyt final
  : public virtual_map_read_onlyt<
      irep_idt,
      goto_function_templatet<bodyt>,
      goto_function_templatet<bodyt> &>
{
public:
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef typename goto_functions_map_lazyt::key_type key_type;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef typename goto_functions_map_lazyt::mapped_type mapped_type;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches mapped_type
  typedef
    typename goto_functions_map_lazyt::const_mapped_type
    const_mapped_type;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef typename goto_functions_map_lazyt::value_type value_type;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef typename goto_functions_map_lazyt::reference reference;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef typename goto_functions_map_lazyt::size_type size_type;

  typedef typename goto_functions_map_lazyt::keys_ranget keys_ranget;

  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef typename goto_functions_map_lazyt::iterator iterator;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef typename goto_functions_map_lazyt::const_iterator const_iterator;

private:
  typedef std::map<key_type, goto_function_templatet<bodyt>> underlying_mapt;
  mutable underlying_mapt goto_functions;

  goto_modelt &goto_model;
  mutable goto_convert_functionst convert_functions;

public:
  typedef
    std::function<void(
      const irep_idt &function_name,
      goto_functionst::goto_functiont &function)>
    post_process_functiont;
  typedef
    std::function<void(goto_functionst &goto_functions)>
    post_process_functionst;

private:
  post_process_functiont post_process_function;
  post_process_functionst post_process_functions;

private:
  language_filest::lazy_method_mapt &get_lazy_method_map()
  {
    return goto_model.language_files.lazy_method_map;
  }

  const language_filest::lazy_method_mapt &get_lazy_method_map() const
  {
    return goto_model.language_files.lazy_method_map;
  }

public:
  /// Creates a goto_functions_map_lazyt.
  goto_functions_map_lazyt(
      goto_modelt &goto_model,
      post_process_functiont &&post_process_function,
      post_process_functionst &&post_process_functions,
      message_handlert &message_handler)
    : goto_model(goto_model),
      convert_functions(goto_model.symbol_table, message_handler),
      post_process_function(std::move(post_process_function)),
      post_process_functions(std::move(post_process_functions))
  {
  }

  goto_functions_mapt<bodyt> freeze()
  {
    post_process_functions(goto_functions);
    // The object returned here has access to the functions we've already
    // loaded but is frozen in the sense that, with regard to the facility to
    // load new functions, it has let it go.
    return goto_functions_mapt<bodyt>(goto_functions);
  }

  /// Gets the number of functions in the map.
  /// \return The number of functions in the map.
  virtual size_type size() const override
  {
    return get_lazy_method_map().size();
  }

  /// Returns whether the map contains any mappings.
  /// \return Whether the map contains any mappings.
  virtual bool empty() const { return get_lazy_method_map().empty(); }

  /// Gets the names of all functions in the map.
  /// \return The set of names of all functions in the map.
  virtual keys_ranget keys() const override
  {
    return get_lazy_method_map() | boost::adaptors::map_keys;
  }

private:
  template<bool is_const>
  class gen_iteratort:public proxy_iteratort<value_type, is_const>
  {
    friend gen_iteratort<false>;
    friend gen_iteratort<true>;

  public:
    // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
    typedef typename gen_iteratort::value_type value_type;
    // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
    typedef typename gen_iteratort::reference reference;

  private:
    typedef language_filest::lazy_method_mapt::const_iterator lmm_iteratort;

    class proxied_iteratort:public gen_iteratort::proxied_iterator_baset
    {
      friend typename gen_iteratort<true>::proxied_iteratort;

    private:
      const goto_functions_map_lazyt &map;
      lmm_iteratort lmm_it;

      typedef typename proxied_iteratort::baset baset;
      typedef typename proxied_iteratort::const_baset const_baset;

    public:
      proxied_iteratort(
          const goto_functions_map_lazyt &map,
          lmm_iteratort lmm_it)
        : map(map), lmm_it(std::move(lmm_it))
      {
      }

      proxied_iteratort(const proxied_iteratort &other)
        : map(other.map),
          lmm_it(other.lmm_it)
      {
      }

      // Copy-construct from non-const iterator to const iterator
      // Disable this in the non-is_const version using enable_if
      // otherwise it will clash with the copy constructor
      template<bool B = is_const, typename std::enable_if<B>::type* = nullptr>
      proxied_iteratort(
          const typename gen_iteratort<false>::proxied_iteratort &other)
        : map(other.map),
          lmm_it(other.lmm_it)
      {
      }

      virtual baset *clone() override
      {
        return new proxied_iteratort(map, lmm_it);
      }

      virtual const_baset *const_clone() override
      {
        return new typename gen_iteratort<true>::proxied_iteratort(*this);
      }

      virtual void operator++() override { ++lmm_it; }

      virtual reference operator*() override
      {
        return map.ensure_entry_loaded(lmm_it->first);
      }

      virtual bool operator==(const baset &other) override
      {
        return lmm_it == dynamic_cast<const proxied_iteratort &>(other).lmm_it;
      }
    };

  public:
    gen_iteratort(const goto_functions_map_lazyt &map, lmm_iteratort lmm_it)
      : proxy_iteratort<value_type, is_const>(
          std::unique_ptr<proxied_iteratort>(
            new proxied_iteratort(map, std::move(lmm_it))))
    {
    }
  };

public:
  /// Returns an iterator pointing to the first element in the map.
  /// \return An iterator pointing to the first element in the map.
  virtual iterator begin() override
  {
    return gen_iteratort<false>(*this, get_lazy_method_map().begin());
  }

  /// Returns an iterator pointing to the first element in the map.
  /// \return An iterator pointing to the first element in the map.
  virtual const_iterator begin() const override
  {
    return gen_iteratort<true>(*this, get_lazy_method_map().begin());
  }

  /// Returns an iterator pointing beyond the last element in the map.
  /// \return An iterator pointing beyond the last element in the map.
  virtual iterator end() override
  {
    return gen_iteratort<false>(*this, get_lazy_method_map().end());
  }

  /// Returns an iterator pointing beyond the last element in the map.
  /// \return An iterator pointing beyond the last element in the map.
  virtual const_iterator end() const override
  {
    return gen_iteratort<true>(*this, get_lazy_method_map().end());
  }

  /// Find an iterator pointing to an element matching the sought key.
  /// \param key: The key to search for.
  /// \return An iterator pointing to the element that matches the key searched
  ///   for or end() if no such element is found.
  virtual iterator find(const key_type &key) override
  {
    return gen_iteratort<false>(*this, get_lazy_method_map().find(key));
  }

  /// Find an iterator pointing to an element matching the sought key.
  /// \param key: The key to search for.
  /// \return An iterator pointing to the element that matches the key searched
  ///   for or end() if no such element is found.
  virtual const_iterator find(const key_type &key) const override
  {
    return gen_iteratort<true>(*this, get_lazy_method_map().find(key));
  }

  /// Checks whether a given function exists in the map.
  /// \param name: The name of the function to search for.
  /// \return True if the map contains the given function.
  virtual bool contains(const key_type &name) const override
  {
    return get_lazy_method_map().count(name)!=0;
  }

  /// Gets the body for a given function.
  /// \param name: The name of the function to search for.
  /// \return The function body corresponding to the given function.
  virtual const_mapped_type at(const key_type &name) const override
  {
    return ensure_entry_loaded(name).second;
  }

  /// Gets the body for a given function.
  /// \param name: The name of the function to search for.
  /// \return The function body corresponding to the given function.
  virtual mapped_type at(const key_type &name) override
  {
    return ensure_entry_loaded(name).second;
  }

private:
  // This returns a non-const reference, but you should const it before
  // returning it to clients from a const method
  reference ensure_entry_loaded(const key_type &name) const
  {
    typename underlying_mapt::iterator it=goto_functions.find(name);
    if(it!=goto_functions.end())
      return *it;
    // Fill in symbol table entry body
    goto_model.language_files.convert_lazy_method(
      name,
      goto_model.symbol_table);
    // Create goto_functiont
    goto_functionst::goto_functiont function;
    convert_functions.convert_function(name, function);
    // Run function-pass conversions
    post_process_function(name, function);
    // Assign procedure-local location numbers for now
    bodyt &body=function.body;
    body.compute_location_numbers();
    // Add to map
    return *goto_functions.insert(
      std::make_pair(name, std::move(function))).first;
  }
};

#endif
