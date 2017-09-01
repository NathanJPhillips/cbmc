/*******************************************************************\

Author: DiffBlue Limited. All rights reserved.

\*******************************************************************/

/// \file
/// Unit tests for util/proxy_iterator

#include <util/proxy_iterator.h>
#include <catch.hpp>
#include <iterator>


template<typename eltt>
class vector_wrappert
{
public:
  std::vector<eltt> vec;

  template<bool is_const>
  class gen_iteratort:public proxy_iteratort<eltt, is_const>
  {
  public:
    typedef
      proxied_iterator_wrappert<typename std::vector<eltt>::const_iterator>
      proxied_iteratort;

    explicit gen_iteratort(typename std::vector<eltt>::const_iterator it)
      : proxy_iteratort<eltt, is_const>(
          std::unique_ptr<proxied_iteratort>(
            new proxied_iteratort(std::move(it))))
    {
    }
  };

  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef proxy_iteratort<eltt, false> iterator;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef proxy_iteratort<eltt, true> const_iterator;

  const_iterator begin() const { return gen_iteratort<true>(vec.begin()); }
  const_iterator end() const { return gen_iteratort<true>(vec.end()); }
};


TEST_CASE("proxy iterator", "[core][util][proxy_iterator]")
{
  std::vector<int> nums = { 2, 5, 7, 1, 3, 7 };
  vector_wrappert<int> wrapped_nums = { nums };
  // Copy to a vector
  std::vector<int> check;
  for(int x : wrapped_nums)
    check.push_back(x);
  REQUIRE(check == nums);
  // Check length
  REQUIRE(
    std::distance(wrapped_nums.begin(), wrapped_nums.end()) == nums.size());
  // Copying
  vector_wrappert<int>::const_iterator a = wrapped_nums.begin();
  // Copy constructor
  vector_wrappert<int>::const_iterator b = a;
  REQUIRE(b == a);
  // Copy assignment operator
  vector_wrappert<int>::const_iterator c = wrapped_nums.end();
  REQUIRE(c != a);
  c = a;
  REQUIRE(c == a);
  // Move assignement operator
  vector_wrappert<int>::const_iterator d = wrapped_nums.end();
  REQUIRE(d != a);
  d = std::move(a);
  REQUIRE(d == b);
  REQUIRE(a != b);
}
