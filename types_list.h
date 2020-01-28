/*
 * type_list.h
 *
 *  Created on: Jan 27, 2020
 *      Author: facs
 */

#ifndef TYPE_LIST_H_
#define TYPE_LIST_H_

#include <cstddef>
#include <type_traits>
#include <exception>
#include <stdexcept>
#include <utility>

namespace types_list {
/********************************************************
***********	primitive	********************************/

template<typename Type>
struct type {
	using use = Type;
};

template<typename ... Ts>
struct types{ };

template<typename ... Ts>
constexpr std::size_t size(types<Ts...>) {
	return sizeof...(Ts);
}

template<typename ... Ts>
constexpr bool is_empty(types<Ts...> tp) {
	return size(tp) == 0;
}


template<typename Head, typename ... Tail>
constexpr type<Head> head(types<Head, Tail...>){
	return {};
}

template<typename Head, typename ... Tail>
constexpr types<Tail...> tail(types<Head, Tail...>){
	return {};
}

/********************************************************
***********	types == != types ******************************/

template<typename ... Ts, typename ... Us>
constexpr bool operator == (types<Ts...>, types<Us...>) {
	return false;
}

template<typename ... Ts>
constexpr bool operator == (types<Ts...>, types<Ts...>) {
	return true;
}

template<typename ... Ts, typename ... Us>
constexpr bool operator != (types<Ts...>, types<Us...>) {
	return true;
}

template<typename ... Ts>
constexpr bool operator != (types<Ts...>, types<Ts...>) {
	return false;
}

static_assert(types<int, int, int>() == types<int, int, int>());
static_assert(types<int, int, int>() != types<int, float, int>());

/********************************************************
***********	type == != type *****************************/

template<typename T, typename U>
constexpr bool operator == (type<T>, type<U>) {
	return false;
}

template<typename T>
constexpr bool operator == (type<T>, type<T>) {
	return true;
}

template<typename T, typename U>
constexpr bool operator != (type<T>, type<U>) {
	return true;
}

template<typename T>
constexpr bool operator != (type<T>, type<T>) {
	return false;
}

static_assert(head(types<int, float, double>()) == type<int>());
static_assert(tail(types<int, float, double>()) == types<float, double>());

/********************************************************
***********	push ****************************************/
template<typename T, typename ... Ts>
constexpr types<T, Ts...>push_front (type<T>, types<Ts...>) {
	return {};
}

template<typename ... Us, typename ... Ts>
constexpr types<Us..., Ts...>push_front (types<Us...>, types<Ts...>) {
	return {};
}

template<typename T, typename ... Ts>
constexpr types<Ts..., T>push_back (type<T>, types<Ts...>) {
	return {};
}

template<typename ... Us, typename ... Ts>
constexpr types<Ts..., Us...>push_back (types<Us...>, types<Ts...>) {
	return {};
}

static_assert(push_front(type<char>(), types<int, float, double>()) ==
		types<char, int, float, double>());

static_assert(push_front(types<char, double>(), types<int, float, double>()) ==
		types<char, double, int, float, double>());

static_assert(push_back(type<char>(), types<int, float, double>()) ==
		types<int, float, double, char>());

static_assert(push_back(types<char, double>(), types<int, float, double>()) ==
		types<int, float, double, char, double>());

/********************************************************
***********	pop *****************************************/

template<typename T, typename ... Ts>
constexpr types<Ts...>pop_front (types<T, Ts...>) {
	return {};
}

static_assert(pop_front(types<int, float, double>()) == types<float, double>());

/********************************************************
***********	contains, find, find_if ********************/

template<typename T, typename ... Ts>
constexpr bool contains (types<Ts...>) {
	return ( ... || std::is_same_v<T, Ts> );
}

static_assert(contains<int>(types<int, float, double>()));
static_assert(!contains<char>(types<int, float, double>()));

template<typename T, typename ... Ts>
constexpr std::size_t find (types<Ts...> tps) {

	bool array [] = {std::is_same_v<T, Ts>...};

	for(std::size_t i = 0; i < size(tps); ++i) {

		if( array[i] ) {

			return i;

		}

	}

	return size(tps);
}

static_assert(find<int>(types<int, float, double>()) == 0);
static_assert(find<double>(types<int, float, double>()) == 2);

template<template<class...> class Predications, class ... Ts>
struct part_caller {
	template<class ... Us>
	using type = typename Predications<Ts..., Us...>::type;
};

template<template<class...> class Predicate, typename ... Ts>
constexpr std::size_t find_if (types<Ts...> tps) {

	bool array [] = { Predicate<Ts>::value... };

	for(std::size_t i = 0; i < size(tps); ++i) {

		if( array[i] ) {

			return i;

		}

	}

	return size(tps);
}

static_assert(find_if<std::is_pointer>(types<int, float*, double>{}) == 1);
static_assert(find_if<part_caller<std::is_base_of, std::exception>::type>
						(types<std::runtime_error, float, double>{}) == 0);

/********************************************************
***********	..._of **************************************/

template<template<class...> class F, class ... Ts>
constexpr bool any_of (types<Ts...>) {
	return ( ... || F<Ts>::value );
}

template<template<class...> class F, class ... Ts>
constexpr bool all_of (types<Ts...>) {
	return ( ... && F<Ts>::value );
}

template<template<class...> class F, class ... Ts>
constexpr bool none_of (types<Ts...> ts) {
	return !any_of<F>(ts);
}

static_assert(any_of<std::is_pointer>(types<int, float*, double>{}));
static_assert(all_of<std::is_pointer>(types<int*, float*, double*>{}));
static_assert(none_of<std::is_pointer>(types<int, float, double>{}));

/********************************************************
***********	transform ***********************************/
template<template<class...> class F, typename... Ts>
constexpr types<typename F<Ts>::type...> transform(types<Ts...>) {
	return {};
}

static_assert(transform<std::add_pointer>(types<int, float, double>{}) ==
		types<int*, float*, double*>{});

/********************************************************
***********	get *****************************************/

template<std::size_t I, class T>  struct indexed_type {
  static constexpr std::size_t value = I;
  using type = T;
};

template<typename IS, class ... Ts>  struct indexed_types;

template<std::size_t ... Is, class ... Ts>
struct indexed_types<std::index_sequence<Is...>, Ts...> {
	struct type : indexed_type<Is, Ts>...{};
};

template<class... Ts>
using indexed_types_for =
		typename indexed_types<std::index_sequence_for<Ts...>, Ts...>::type;

template<std::size_t I, class T>
constexpr type<T> get_indexed_type(indexed_type<I,T>) { return {}; }

template<std::size_t I, class... Ts>
constexpr auto get(types<Ts...>) {
	return get_indexed_type<I>(indexed_types_for<Ts...>{});
}


static_assert(get<1>(types<double, int, char>{}) == type<int>{});


}
#endif /* TYPE_LIST_H_ */

