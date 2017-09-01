/*******************************************************************\

Author: Nathan.Phillips@diffblue.com

\*******************************************************************/


/// \file
/// proxy_iteratort derivative that wraps a normal iterator

#ifndef CPROVER_UTIL_PROXY_WRAPPED_ITERATOR_H
#define CPROVER_UTIL_PROXY_WRAPPED_ITERATOR_H

#include "proxy_iterator.h"


template<typename iteratort, typename const_iteratort = iteratort>
class proxy_wrapped_iteratort
  : public proxy_iteratort<
      typename iteratort::value_type,
      std::is_same<iteratort, const_iteratort>::value>
{
private:
  static constexpr bool is_const =
    std::is_same<iteratort, const_iteratort>::value;

public:
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef typename proxy_wrapped_iteratort::value_type value_type;

public:   // Not ideal but required to enable const_clone
  class wrapped_iteratort
    : public proxy_wrapped_iteratort::proxied_iterator_baset
  {
  private:
    iteratort it;

    typedef typename wrapped_iteratort::baset baset;
    typedef typename wrapped_iteratort::const_baset const_baset;

  public:
    // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
    typedef typename iteratort::reference reference;

    explicit wrapped_iteratort(iteratort it)
      : it(std::move(it))
    {
    }

    virtual baset *clone() override
    {
      return new wrapped_iteratort(it);
    }

    virtual const_baset *const_clone() override
    {
      return new
        typename proxy_wrapped_iteratort<const_iteratort>::wrapped_iteratort(
          it);
    }

    virtual void operator++() override { ++it; }

    virtual reference operator*() override { return *it; }

    virtual bool operator==(const baset &other) override
    {
      return it == dynamic_cast<const wrapped_iteratort &>(other).it;
    }
  };

public:
  explicit proxy_wrapped_iteratort(iteratort it)
    : proxy_iteratort<value_type, is_const>(
        std::unique_ptr<wrapped_iteratort>(
          new wrapped_iteratort(std::move(it))))
  {
  }
};

#endif
