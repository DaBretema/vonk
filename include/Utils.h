#pragma once

#include <algorithm>

#include <set>
#include <vector>
#include <unordered_map>

namespace vo::algo
{
template<typename T1, typename T2>
std::vector<T1> mapKeys(std::unordered_map<T1, T2> m)
{
  std::vector<T1> o;
  o.reserve(m.size());
  for (auto &&[k, v] : m) { o.push_back(k); }
  std::reverse(o.begin(), o.end());
  return o;
}
template<typename T1, typename T2>
std::vector<T2> mapVals(std::unordered_map<T1, T2> m)
{
  std::vector<T2> o;
  o.reserve(m.size());
  for (auto &&[k, v] : m) { o.push_back(v); }
  std::reverse(o.begin(), o.end());
  return o;
}
template<typename T1, typename T2>
std::set<T2> mapValsUnique(std::unordered_map<T1, T2> m)
{
  std::vector<T2> o;
  o.reserve(m.size());
  for (auto &&[k, v] : m) { o.push_back(v); }
  std::reverse(o.begin(), o.end());
  return std::set { o.begin(), o.end() };
}

}  // namespace vo::algo

namespace vo::files
{
std::vector<char> read(std::string const &filepath);
}  // namespace vo::files
