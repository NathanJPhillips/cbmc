/*******************************************************************\

Author: Nathan.Phillips@diffblue.com

\*******************************************************************/


/// \file
/// Iterator that is a proxy for a proxied implementation
/// Enables iterators that have virtual members for their implementation

#ifndef CPROVER_UTIL_PROXY_ITERATOR_H
#define CPROVER_UTIL_PROXY_ITERATOR_H

#include "type_traits.h"
#include <memory>


template<typename valuet, bool is_const>
class proxy_iteratort
{
  friend proxy_iteratort<valuet, false>;
  friend proxy_iteratort<valuet, true>;

private:
  // Utility to add const if is_const is true
  template<typename BaseType>
  using constify = conditionally_make_referand_const<is_const, BaseType>;

public:
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef std::forward_iterator_tag iterator_category;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef valuet value_type;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef constify<value_type> & reference;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef constify<value_type> * pointer;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef std::ptrdiff_t difference_type;

protected:
  class proxied_iterator_baset
  {
  protected:
    // Typedefs for base and const version of base
    typedef proxied_iterator_baset baset;
    typedef
      typename proxy_iteratort<value_type, true>::proxied_iterator_baset
      const_baset;

  public:
    virtual baset *clone()=0;
    virtual const_baset *const_clone()=0;
    virtual void operator++()=0;
    virtual reference operator*()=0;
    virtual bool operator==(const baset &other)=0;
  };

private:
  typedef std::unique_ptr<proxied_iterator_baset> proxied_iterator_ptrt;
  proxied_iterator_ptrt it;

protected:
  explicit proxy_iteratort(proxied_iterator_ptrt it)
    : it(std::move(it))
  {
  }

public:
  // This creates an invalid iterator that should never be used
  // If you are using this then you should really be using an optional iterator
  //  instead via something like boost::optional
  proxy_iteratort() { }

  // Copy constructor
  proxy_iteratort(const proxy_iteratort &other)
    : it(other.it->clone())
  {
  }
  proxy_iteratort &operator=(
    const proxy_iteratort &other)
  {
    // Create a temporary using the copy constructor and move-assign it to this
    return *this = proxy_iteratort(other);
  }
  // Copy-construct from non-const iterator to const iterator
  // Disable this in the non-is_const version using enable_if
  // otherwise it will clash with the copy constructor
  template<bool B = is_const, typename std::enable_if<B>::type* = nullptr>
  proxy_iteratort(const proxy_iteratort<value_type, false> &other)
    : it(other.it->const_clone())
  {
  }

public:
  proxy_iteratort(proxy_iteratort &&other)=default;
  proxy_iteratort &operator=(
    proxy_iteratort &&other)=default;

public:
  proxy_iteratort &operator++()
  {
    ++(*it);
    return *this;
  }

  proxy_iteratort operator++(int)
  {
    proxy_iteratort me(*this);
    ++(*this);
    return me;
  }

  reference operator*() const
  {
    return **it;
  }

  pointer operator->() const
  {
    return &this->operator*();
  }

  friend bool operator==(const proxy_iteratort &x, const proxy_iteratort & y)
  {
    return *x.it == *y.it;
  }

  friend bool operator!=(const proxy_iteratort &x, const proxy_iteratort & y)
  {
    return !(x == y);
  }
};

#endif
