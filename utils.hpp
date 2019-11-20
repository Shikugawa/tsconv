#pragma once

#include <vector>

namespace TSConv {
template <class T> T FindReplace(T s, T s1, T s2) {
  auto pos = s.find(s1);
  auto len = s1.length();
  if (pos != std::string::npos) {
    s.replace(pos, len, s2);
  }
  return s;
}

template <class T> T Join(const std::vector<T> &ary) {
  T s;
  for (const auto &a : ary) {
    s += a;
  }
  return s;
}
} // namespace TSConv
