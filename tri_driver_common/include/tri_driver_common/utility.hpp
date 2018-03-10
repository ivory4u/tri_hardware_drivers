#pragma once

#include <stdlib.h>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <iostream>

// Branch prediction hints
// Figure out which compiler we have
#if defined(__clang__)
  /* Clang/LLVM */
  #define LIKELY(x) __builtin_expect(!!(x), 1)
  #define UNLIKELY(x) __builtin_expect(!!(x), 0)
#elif defined(__ICC) || defined(__INTEL_COMPILER)
  /* Intel ICC/ICPC */
  #define LIKELY(x) __builtin_expect(!!(x), 1)
  #define UNLIKELY(x) __builtin_expect(!!(x), 0)
#elif defined(__GNUC__) || defined(__GNUG__)
  /* GNU GCC/G++ */
  #define LIKELY(x) __builtin_expect(!!(x), 1)
  #define UNLIKELY(x) __builtin_expect(!!(x), 0)
#elif defined(_MSC_VER)
  /* Microsoft Visual Studio */
  /* MSVC doesn't support branch prediction hints. Use PGO instead. */
  #define LIKELY(x) (x)
  #define UNLIKELY(x) (x)
#endif

// Macro to disable unused parameter compiler warnings
#define UNUSED(x) (void)(x)

namespace tri_driver_common
{
namespace utility
{
template <typename T>
inline T ClampValue(const T& val, const T& min, const T& max)
{
  if (max >= min)
  {
    return std::min(max, std::max(min, val));
  }
  else
  {
    throw std::invalid_argument("min > max");
  }
}

template <class T>
inline T ClampValueAndWarn(const T& val, const T& min, const T& max)
{
  if (max >= min)
  {
    if (val < min)
    {
      const std::string msg = "Clamping " + std::to_string(val)
                              + " to min " + std::to_string(min) + "\n";
      std::cerr << msg << std::flush;
      return min;
    }
    else if (val > max)
    {
      const std::string msg = "Clamping " + std::to_string(val)
                              + " to max " + std::to_string(max) + "\n";
      std::cerr << msg << std::flush;
      return max;
    }
    return val;
  }
  else
  {
    throw std::invalid_argument("min > max");
  }
}

template<typename T>
inline bool CheckAlignment(const T& item,
                           const uint64_t desired_alignment,
                           bool verbose=false)
{
  const T* item_ptr = &item;
  const uintptr_t item_ptr_val = (uintptr_t)item_ptr;
  if ((item_ptr_val % desired_alignment) == 0)
  {
    if (verbose)
    {
      const std::string msg = "Item @ " + std::to_string(item_ptr_val)
                              + " aligned to "
                              + std::to_string(desired_alignment) + " bytes\n";
      std::cout << msg << std::flush;
    }
    return true;
  }
  else
  {
    if (verbose)
    {
      const std::string msg = "Item @ " + std::to_string(item_ptr_val)
                              + " NOT aligned to "
                              + std::to_string(desired_alignment) + " bytes\n";
      std::cout << msg << std::flush;
    }
    return false;
  }
}

template<typename T>
inline void RequireAlignment(const T& item, const uint64_t desired_alignment)
{
  const T* item_ptr = &item;
  const uintptr_t item_ptr_val = (uintptr_t)item_ptr;
  if ((item_ptr_val % desired_alignment) != 0)
  {
    throw std::runtime_error("Item @ " + std::to_string(item_ptr_val)
                             + " not aligned at desired alignment of "
                             + std::to_string(desired_alignment) + " bytes");
  }
}

template <typename T>
inline T SetBit(const T current,
                const uint32_t bit_position,
                const bool bit_value)
{
  // Safety check on the type we've been called with
  static_assert((std::is_same<T, uint8_t>::value
           || std::is_same<T, uint16_t>::value
           || std::is_same<T, uint32_t>::value
           || std::is_same<T, uint64_t>::value),
          "Type must be a fixed-size unsigned integral type");
  // Do it
  T update_mask = 1;
  update_mask = update_mask << bit_position;
  if (bit_value)
  {
    return (current | update_mask);
  }
  else
  {
    update_mask = (~update_mask);
    return (current & update_mask);
  }
}

template <typename T>
inline bool GetBit(const T current, const uint32_t bit_position)
{
  // Type safety checks are performed in the SetBit() function
  const uint32_t mask = SetBit((T)0, bit_position, true);
  if ((mask & current) > 0)
  {
    return true;
  }
  else
  {
    return false;
  }
}

template <typename Key, typename Value,
          typename Compare=std::less<Key>,
          typename Allocator=std::allocator<std::pair<const Key, Value>>>
inline Value RetrieveOrDefault(
    const std::map<Key, Value, Compare, Allocator>& map,
    const Key& key,
    const Value& default_val)
{
  const auto found_itr = map.find(key);
  if (found_itr != map.end())
  {
    return found_itr->second;
  }
  else
  {
    return default_val;
  }
}

template <typename Key, typename Value,
          typename Hash=std::hash<Key>,
          typename Predicate=std::equal_to<Key>,
          typename Allocator=std::allocator<std::pair<const Key, Value>>>
inline Value RetrieveOrDefault(
    const std::unordered_map<Key, Value, Hash, Predicate, Allocator>& map,
    const Key& key,
    const Value& default_val)
{
  const auto found_itr = map.find(key);
  if (found_itr != map.end())
  {
    return found_itr->second;
  }
  else
  {
    return default_val;
  }
}

inline void ConditionalPrint(const std::string& msg,
                             const int32_t msg_level,
                             const int32_t print_level)
{
  if (UNLIKELY(msg_level <= print_level))
  {
    const std::string printstr = "[" + std::to_string(msg_level) + "/"
                                 + std::to_string(print_level) + "] "
                                 + msg + "\n";
    std::cout << printstr << std::flush;
  }
}

inline void ConditionalError(const std::string& msg,
                             const int32_t msg_level,
                             const int32_t print_level)
{
  if (UNLIKELY(msg_level <= print_level))
  {
    const std::string printstr = "[" + std::to_string(msg_level) + "/"
                                 + std::to_string(print_level) + "] "
                                 + msg + "\n";
    std::cerr << printstr << std::flush;
  }
}

inline bool CheckAllStringsForSubstring(const std::vector<std::string>& strings,
                                        const std::string& substring)
{
  for (size_t idx = 0; idx < strings.size(); idx++)
  {
    const std::string& candidate_string = strings[idx];
    const size_t found = candidate_string.find(substring);
    if (found == std::string::npos)
    {
      return false;
    }
  }
  return true;
}

template <typename T, typename Alloc=std::allocator<T>>
inline bool IsSubset(const std::vector<T, Alloc>& set,
                     const std::vector<T, Alloc>& candidate_subset)
{
  std::map<T, uint8_t> items_map;
  for (size_t idx = 0; idx < set.size(); idx++)
  {
    const T& item = set[idx];
    items_map[item] = 0x01;
  }
  for (size_t idx = 0; idx < candidate_subset.size(); idx++)
  {
    const T& item = candidate_subset[idx];
    const auto found_itr = items_map.find(item);
    if (found_itr == items_map.end())
    {
      return false;
    }
  }
  return true;
}

template <typename T, typename Alloc=std::allocator<T>>
inline bool SetsEqual(const std::vector<T, Alloc>& set1,
                      const std::vector<T, Alloc>& set2)
{
  if (IsSubset(set1, set2) && IsSubset(set2, set1))
  {
    return true;
  }
  else
  {
    return false;
  }
}

template <typename Key, typename Value,
          typename Compare=std::less<Key>,
          typename Allocator=std::allocator<std::pair<const Key, Value>>>
inline std::vector<Key> GetKeys(
    const std::map<Key, Value, Compare, Allocator>& map)
{
  std::vector<Key> keys;
  keys.reserve(map.size());
  typename std::map<Key, Value, Compare, Allocator>::const_iterator itr;
  for (itr = map.begin(); itr != map.end(); ++itr)
  {
    const Key cur_key = itr->first;
    keys.push_back(cur_key);
  }
  keys.shrink_to_fit();
  return keys;
}

template <typename Key, typename Value,
          typename Compare=std::less<Key>,
          typename Allocator=std::allocator<std::pair<const Key, Value>>>
inline std::vector<std::pair<const Key, Value>, Allocator> GetKeysAndValues(
    const std::map<Key, Value, Compare, Allocator>& map)
{
  std::vector<std::pair<const Key, Value>, Allocator> keys_and_values;
  keys_and_values.reserve(map.size());
  typename std::map<Key, Value, Compare, Allocator>::const_iterator itr;
  for (itr = map.begin(); itr != map.end(); ++itr)
  {
    const std::pair<Key, Value> cur_pair(itr->first, itr->second);
    keys_and_values.push_back(cur_pair);
  }
  keys_and_values.shrink_to_fit();
  return keys_and_values;
}

template <typename Key, typename Value,
          typename Compare=std::less<Key>,
          typename Allocator=std::allocator<std::pair<const Key, Value>>>
inline std::map<Key, Value, Compare, Allocator> MakeFromKeysAndValues(
    const std::vector<std::pair<const Key, Value>, Allocator>& keys_and_values)
{
  std::map<Key, Value, Compare, Allocator> map;
  for (size_t idx = 0; idx < keys_and_values.size(); idx++)
  {
    const std::pair<Key, Value>& cur_pair = keys_and_values[idx];
    map[cur_pair.first] = cur_pair.second;
  }
  return map;
}

template <typename Key, typename Value,
          typename Compare=std::less<Key>,
          typename KeyVectorAllocator=std::allocator<Key>,
          typename ValueVectorAllocator=std::allocator<Value>,
          typename PairAllocator=std::allocator<std::pair<const Key, Value>>>
inline std::map<Key, Value, Compare, PairAllocator> MakeFromKeysAndValues(
    const std::vector<Key, KeyVectorAllocator>& keys,
    const std::vector<Value, ValueVectorAllocator>& values)
{
  assert(keys.size() == values.size());
  std::map<Key, Value, Compare, PairAllocator> map;
  for (size_t idx = 0; idx < keys.size(); idx++)
  {
    const Key& cur_key = keys[idx];
    const Value& cur_value = values[idx];
    map[cur_key] = cur_value;
  }
  return map;
}
}
}
