/*
 * The MIT License
 *
 * Copyright 2019 Mihail Slobodyanuk <slobodyanukma@gmail.com>.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
/* 
 * File:   rpc.hpp
 * Author: Mihail Slobodyanuk <slobodyanukma@gmail.com>
 *
 * Created on February 1, 2019, 8:56 AM
 */

#ifndef RPC_HPP
#define RPC_HPP

#include <string>
#include <tuple>

namespace rpc {

	namespace detail {

		template<class ReturnType, class... ArgsType>
		struct func_meta_t {
			typedef ReturnType(*func_type)(ArgsType...);

			func_meta_t(func_type f)
			: m_address(f) {
			}
			func_type m_address;
		};

		template<class R, class... A>
		func_meta_t<R, A...> make_func_meta(R(*f)(A...)) {
			return func_meta_t<R, A...>(f);
		}

		template <class F, class Tuple, std::size_t... I>
		constexpr decltype(auto) apply_impl(F&& f, Tuple&& t, std::index_sequence<I...>) {
			return f(std::get<I>(std::forward<Tuple>(t))...);
		}
	}

	template<class Serializer, class Ipc, class ... FuncMetas>
	struct rpc_t {
		typedef typename Serializer::ibuffer_t ibuffer_t;
		typedef typename Serializer::obuffer_t obuffer_t;
		typedef Ipc ipc_t;
		typedef std::tuple<FuncMetas...> registry_t;
		typedef std::tuple_size<registry_t> registry_size;
		ipc_t ipc;
		const registry_t registry;

		rpc_t(FuncMetas&& ... fm) : registry{fm ...}
		{
		}

		rpc_t(ipc_t&& ipc, FuncMetas&& ... fm) : ipc{ipc}, registry{fm ...}
		{
		}

		template<class R, class... A>
		std::string marshal_strong(R(*f)(A...), A&& ... as) {
			obuffer_t buffer;
			buffer << find_function_index(f);
			append_arguments(buffer, as...);
			return buffer.str();
		}

		//Do implicit arguments type conversion if possible

		template<class R, class... A, class... A1>
		std::string marshal(R(*f)(A...), A1&& ... as) {
			return marshal_strong(f, std::forward<A>(as)...);
		}

		std::string invoke(std::string callStr) {
			ibuffer_t buffer(callStr);
			size_t functionIndex;
			buffer >> functionIndex;
			obuffer_t response;
			apply_function_by_index(functionIndex, buffer, response);
			return response.str();
		}

		template<class R, class... A, class... A1>
		R operator()(R(*f)(A...), A1&& ... as) {
			std::string callStr = marshal_strong(f, std::forward<A>(as)...);
			ipc.send(callStr);
			std::string respStr = ipc.recv();
			//Unmarshall
			ibuffer_t buffer(respStr);
			R result;
			buffer >> result;
			return result;
		}

		template<class... A, class... A1>
		void operator()(void(*f)(A...), A1&& ... as) {
			std::string callStr = marshal(f, std::forward<A>(as)...);
			ipc.send(callStr);
			ipc.recv();
		}
		
		void listen() {
			while(true) {
				ipc.send(invoke(ipc.recv()));
			}
		}
	private:
		//Function not found in the registry

		template<std::size_t I = 0, class F >
		int find_function_index(F, typename std::enable_if<I == registry_size::value>::type* = 0) {
			throw std::out_of_range("The call is not registered");
		}

		template<std::size_t I = 0, class F >
		int find_function_index(F f, typename std::enable_if<I < registry_size::value>::type* = 0
						) {
			return find_function_index < I + 1 > (f);
		}

		template<std::size_t I = 0 >
		int find_function_index(typename std::tuple_element<I, registry_t>::type::func_type f, typename std::enable_if<I < registry_size::value>::type* = 0
						) {
			if (std::get<I>(registry).m_address == f) {
				return I;
			} else {
				return find_function_index < I + 1 > (f);
			}
		}

		void append_arguments(obuffer_t&) {
		}

		template<class Arg>
		void append_arguments(obuffer_t& args, Arg&& arg) {
			args << arg;
		}

		template<class Arg0, class ... Args>
		void append_arguments(obuffer_t& obuffer, Arg0&& arg0, Args&& ... args) {
			append_arguments(obuffer, std::forward<Arg0>(arg0));
			append_arguments(obuffer, std::forward<Args>(args)...);
		}

		template <class Tuple, class R, class ... A>
		void apply(R(*f)(A...), Tuple&& t, obuffer_t& response, typename std::enable_if<!std::is_void< R >::value>::type* = 0) {
			response << detail::apply_impl(
							f, std::forward<Tuple>(t),
							std::make_index_sequence<std::tuple_size<std::remove_reference_t < Tuple>>::value>
			{
			});
		}

		template <class Tuple, class R, class ... A>
		void apply(R(*f)(A...), Tuple&& t, obuffer_t&, typename std::enable_if<std::is_void< R >::value>::type* = 0) {
			detail::apply_impl(
							f, std::forward<Tuple>(t),
							std::make_index_sequence<std::tuple_size<std::remove_reference_t < Tuple>>::value>
			{
			});
		}

		//End of recursion stub

		template <std::size_t I = 0, typename Tp>
		void fill_args_tuple(Tp&, ibuffer_t&, typename std::enable_if<I == std::tuple_size<Tp>::value>::type* = 0) {
		}

		template <std::size_t I = 0, typename Tp>
		void fill_args_tuple(Tp& t, ibuffer_t& args, typename std::enable_if<I < std::tuple_size<Tp>::value>::type* = 0) {
			args >> std::get<I>(t);
			fill_args_tuple < I + 1, Tp > (t, args);
		}

		template<class R, class... A>
		void apply(R(*f)(A...), ibuffer_t& args, obuffer_t& response) {
			auto tArgs = std::make_tuple(std::decay_t<A>()...);
			fill_args_tuple(tArgs, args);
			apply(f, tArgs, response);
		}

		template<std::size_t I>
		void apply_function_by_index(size_t, ibuffer_t&, obuffer_t&, typename std::enable_if<I == registry_size::value>::type* = 0) {
			throw std::out_of_range("The call is not registered");
		}

		template<std::size_t I = 0 >
		void apply_function_by_index(size_t i, ibuffer_t& args, obuffer_t& response, typename std::enable_if<I < registry_size::value>::type* = 0) {
			if (i == I) {
				apply(std::get<I>(registry).m_address, args, response);
			} else {
				apply_function_by_index < I + 1 > (i, args, response);
			}
		}
	};

	namespace detail {

		template<class Serializer, class Ipc, class... F>
		auto make_rpc_wrapper(F&&... f) {
			return rpc_t<Serializer, Ipc, F...>(std::forward<F>(f)...);
		}

		template<class Serializer, class Ipc, class... F>
		auto make_rpc_wrapper(Ipc&& ipc, F&&... f) {
			return rpc_t<Serializer, Ipc, F...>(std::forward<Ipc>(ipc), std::forward<F>(f)...);
		}
	}

	template<class Serializer, class Ipc, class... F>
	auto make_rpc(F... f) {
		return detail::make_rpc_wrapper<Serializer, Ipc>(detail::make_func_meta(f)...);
	}

	template<class Serializer, class Ipc, class... F>
	auto make_rpc(Ipc&& ipc, F... f) {
		return detail::make_rpc_wrapper<Serializer, Ipc>(std::forward<Ipc>(ipc), detail::make_func_meta(f)...);
	}
}

#endif /* RPC_HPP */

