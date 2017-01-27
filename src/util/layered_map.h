#pragma once

#include <cassert>
#include <memory>
#include <set>
#include <map>


template<typename Key, typename Value>
class empty_layered_mapt
{
  private:
    empty_layered_mapt() { }

  public:
    static std::shared_ptr<empty_layered_mapt<Key, Value>> create()
    {
      return std::shared_ptr<empty_layered_mapt<Key, Value>>(
        new empty_layered_mapt<Key, Value>);
    }

    class const_iterator
    {
      public:
        std::pair<Key, Value> operator*() const { throw; }

        bool operator==(const const_iterator &other) const { return true; }
        bool operator!=(const const_iterator &other) const { return false; }
    };

    const_iterator begin() const
    {
      return const_iterator();
    }

    const_iterator end() const
    {
      return const_iterator();
    }

    const_iterator find(const Key &key) const
    {
      return const_iterator();
    }
};

template<typename Key, typename Value, typename Map_Baset = empty_layered_mapt<Key, Value>>
class layered_mapt
{
  private:
    using type = layered_mapt<Key, Value, Map_Baset>;

    std::shared_ptr<Map_Baset> base_map;
    std::set<Key> removals;
    std::map<Key, Value> local_map;

    std::shared_ptr<type> self;

  public:
    using size_type = typename std::map<Key, Value>::size_type;

    class const_iterator
    {
      private:
        typename Map_Baset::const_iterator base_iterator;
        const typename Map_Baset::const_iterator base_end;
        const std::set<Key> &removals;
        typename std::map<Key, Value>::const_iterator local_iterator;
        std::map<Key, Value> &local_map;

      public:
        const_iterator(type map)
          : base_iterator(map.base_map->begin()), base_end(map.base_map->end()),
            removals(map.removals),
            local_iterator(map.local_map.begin()), local_map(map.local_map)
        {
          skip_overriden_elements();
        }

        const_iterator(type map,
            typename Map_Baset::const_iterator base_iterator,
            typename std::map<Key, Value>::const_iterator local_iterator)
          : base_iterator(base_iterator), base_end(map.base_map->end()),
            removals(map.removals),
            local_iterator(local_iterator), local_map(map.local_map)
        {
        }

        const_iterator(type map,
            typename Map_Baset::const_iterator base_iterator)
          : base_iterator(base_iterator), base_end(map.base_map->end()),
            removals(map.removals),
            local_iterator(map.local_map.begin()), local_map(map.local_map)
        {
        }

        const_iterator(type map,
            typename std::map<Key, Value>::const_iterator local_iterator)
          : base_iterator(map.base_map->end()), base_end(map.base_map->end()),
            removals(map.removals),
            local_iterator(local_iterator), local_map(map.local_map)
        {
        }

      private:
        void skip_overriden_elements()
        {
          while(base_iterator!=base_end
            && removals.find(base_iterator->first)!=removals.end()
            && local_map.find(base_iterator->first)!=local_map.end())
          {
            ++base_iterator;
          }
        }

      public:
        const_iterator& operator++()
        {
          if(base_iterator!=base_end)
          {
            ++base_iterator;
            skip_overriden_elements();
          }
          else if(local_iterator!=local_map.end())
            ++local_iterator;
          return this;
        }
        const_iterator operator++(int)
        {
          const_iterator retval = *this;
          ++(*this);
          return retval;
        }

        std::pair<Key, Value> operator*() const
        {
          if(base_iterator!=base_end)
            return *base_iterator;
          return *local_iterator;
        }

        bool operator==(const const_iterator &other) const
        {
          return base_iterator == other.base_iterator
            && local_iterator == other.local_iterator;
        }
        bool operator!=(const const_iterator &other) const
        {
          return !(*this == other);
        }
    };

  private:
    layered_mapt(std::shared_ptr<Map_Baset> base_map)
      : base_map(base_map)
    {
      assert(base_map);
    }

    layered_mapt()
      : base_map(empty_layered_mapt<Key, Value>::create())
    {
    }

    static std::shared_ptr<type> create(std::shared_ptr<Map_Baset> base_map)
    {
      std::shared_ptr<type> new_ptr(new type(base_map));
      new_ptr->self = new_ptr;
      return new_ptr;
    }
    friend Map_Baset;

  public:
    static std::shared_ptr<type> create()
    {
      std::shared_ptr<type> new_ptr = std::shared_ptr<type>(new type);
      new_ptr->self = new_ptr;
      return new_ptr;
    }

    const_iterator begin() const
    {
      return const_iterator();
    }

    const_iterator end() const
    {
      return const_iterator(*this, base_map->end(), local_map.end());
    }

    const_iterator find(const Key &key) const
    {
      typename std::map<Key, Value>::const_iterator local_result = local_map.find(key);
      if(local_result!=local_map.end())
        return const_iterator(*this, local_result);
      if(removals.find(key)!=removals.end())
        return end();
      typename Map_Baset::const_iterator base_result = base_map->find(key);
      if(base_result!=base_map->end())
        return const_iterator(*this, base_result);
      return end();
    }

    std::pair<const_iterator, bool> insert(const Key &key, const Value &value)
    {
      const_iterator it = find(key);
      if(it!=end())
        return std::make_pair(it, false);
      return std::make_pair(
        const_iterator(*this, local_map.insert(make_pair(key, value)).first),
        true);
    }

    size_type erase(const Key &key)
    {
      if(local_map.erase(key))
        return 1;
      if(removals.find(key)!=removals.end())
        return 0;
      if(base_map->find(key)!=base_map->end())
      {
        removals.insert(key);
        return 1;
      }
      return 0;
    }

    std::shared_ptr<layered_mapt<Key, Value, type>> freeze() const
    {
      return layered_mapt<Key, Value, type>::create(self);
    }
};
