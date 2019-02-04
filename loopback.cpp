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
 * File:   loopback.cpp
 * Author: Mihail Slobodyanuk <slobodyanukma@gmail.com>
 *
 * Created on February 4, 2019, 7:15 AM
 */

#include <stdlib.h>
#include <iostream>
#include <functional>
#include "rpc.hpp"
#include "rpc_streams.hpp"
#include "my_interface.h"

struct delegate_ipc_t {
	std::function<void(std::string) > on_send;
	std::function<std::string() > on_recv;

	void send(std::string str) {
		on_send(str);
	}

	std::string recv() {
		return on_recv();
	}
};

int main() {
	typedef void(*lambda_t)();
	lambda_t lambda = [] {
		std::cout << __PRETTY_FUNCTION__ << std::endl;
	};

	auto myrpc = rpc::make_rpc<rpc::stream_serializer, delegate_ipc_t>(
					no_args
					, one_arg
					, many_args
					, add
					, lambda
					, exit
					);
	std::string message;
	myrpc.ipc.on_send = [&](const std::string & data) {
		message = myrpc.invoke(data);
		std::cerr << "SENT: " << data << std::endl;
	};
	myrpc.ipc.on_recv = [&]() {
		std::cerr << "RECV: " << message << std::endl;
		return message;
	};


	std::string const hello{"hello"};
	std::string const esc_string{"String with spaces, percents %, tab \t and new line \r\n"};
	const char* c_str = "zero terminated";
	myrpc(no_args);
	myrpc(one_arg, hello);
	myrpc(many_args, esc_string, 2, c_str, "string literal");
	std::cerr << myrpc(add, 1, 2) << std::endl;
	myrpc(lambda);
	myrpc(exit, 0);

	return 0;
}

