//
// Copyright (C) 2019-2019 Dominik S. Buse <buse@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
#pragma once

#include <functional>
#include <tuple>
#include <type_traits>

/*
 * Compile-Time introspection into callables (function pointers, lambdas, functors, etc.)
 *
 * With this helper, one can identify the return type and individual arguments of any callable.
 * This allows to convert them info regular std::function types, even in the presence of templated arguments.
 */
namespace veins {
namespace CallableInfo {
template <class Ret, class Cls, class ... Args>
struct trait {
    static constexpr size_t arity = sizeof...(Args);
    using return_type = Ret;
    using function_equivalent = std::function<Ret(Args...)>;

    template <size_t i>
    struct arg {
        using type = typename std::tuple_element<i, std::tuple<Args...>>::type;
    };
};

// crutch to break up recursive relation that makes doxygen throw errors.
template <class Lambda>
struct lambda_type;

// convenience call for lambdas
template <class Lambda>
struct type : lambda_type<Lambda> {
};

template <class Lambda>
struct lambda_type : type<decltype(&Lambda::operator())> {
};

// for normal functions / function pointer
template <class Ret, class ... Args>
struct type<Ret (*)(Args...)> : trait<Ret, Ret (*)(Args...), Args...> {
};

// for normal (const) lambdas
template <class Ret, class Cls, class ... Args>
struct type<Ret (Cls::*)(Args...) const> : trait<Ret, Cls, Args...> {
};

// for mutable lambdas
template <class Ret, class Cls, class ... Args>
struct type<Ret (Cls::*)(Args...)> : trait<Ret, Cls, Args...> {
};
}; // namespace CallableInfo
}; // namespace veins
