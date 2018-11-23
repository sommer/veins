#pragma once

#include <memory>
#include <utility>

#ifdef __cpp_lib_make_unique
using make_unique = std::make_unique;
#else
/**
 * User-defined implementation of std::make_unique.
 *
 * Until Veins builds on C++14 this provides equivalent functionality.
 */
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#endif
