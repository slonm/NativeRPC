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
 * File:   sstream_buffers.h
 * Author: Mihail Slobodyanuk <slobodyanukma@gmail.com>
 *
 * Created on February 2, 2019, 8:00 AM
 */

#ifndef SSTREAM_BUFFERS_H
#define SSTREAM_BUFFERS_H

#include <iostream>
#include <sstream>
#include <iomanip>
#include <memory>
#include <vector>

namespace rpc {

	struct ibuffer_t {
		std::istringstream is;
		std::vector<std::unique_ptr<std::string>> strings;

		std::string decode(const std::string& in) {
			std::string out;
			out.reserve(in.size());
			for (size_t i = 0; i < in.size(); ++i) {
				if (in[i] == '%') {
					if (i + 3 <= in.size()) {
						int value = 0;
						std::istringstream is(in.substr(i + 1, 2));
						if (is >> std::hex >> value) {
							out += static_cast<char> (value);
							i += 2;
						} else {
							throw std::runtime_error("bad encoding");
						}
					} else {
						throw std::runtime_error("bad encoding");
					}
				} else {
					out += in[i];
				}
			}
			return out;
		}

		ibuffer_t(const std::string& str) : is(str) {
			//			is.exceptions(std::stringstream::eofbit);
			//			tmpIs.exceptions(std::stringstream::eofbit);
		}

		template<class T>
		ibuffer_t& operator>>(T& t) {
			is >> t;
			return *this;
		}

		ibuffer_t& operator>>(const char*& t) {
			strings.emplace_back(new std::string);
			is >> *strings.back();
			*strings.back() = decode(*strings.back());
			t = strings.back()->c_str();
			return *this;
		}

		ibuffer_t& operator>>(std::string& t) {
			is >> t;
			t = decode(t);
			return *this;
		}
	};

	struct obuffer_t {
		std::ostringstream os;

		std::ostringstream& push_encoded(const std::string& url) {
			for (auto& c : url) {
				switch (c) {
					case ' ':
					case '\t':
					case '\r':
					case '\n':
					case '%':
					{
						os << '%' << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << int(static_cast<unsigned char> (c));
					}
						break;
					default:
						os << c;
				}
			}
			return os;
		}

		obuffer_t() {
			os.exceptions(std::stringstream::failbit | std::stringstream::badbit | std::stringstream::eofbit);
		}

		obuffer_t(const std::string& str) : os(str) {
			os.exceptions(std::stringstream::failbit | std::stringstream::badbit | std::stringstream::eofbit);
		}

		template<class T>
		obuffer_t& operator<<(const T& t) {
			os << t << " ";
			return *this;
		}

		obuffer_t& operator<<(const char*& t) {
			push_encoded(t) << " ";
			return *this;
		}

		obuffer_t& operator<<(const std::string& t) {
			push_encoded(t) << " ";
			return *this;
		}

		std::string str() {
			auto result = os.str();
			return result.substr(0, result.size() - 1);
		}
	};

	struct stream_serializer {
		using ibuffer_t = rpc::ibuffer_t;
		using obuffer_t = rpc::obuffer_t;
	};

	struct stdin_stdout_ipc_t {

		void send(std::string str) {
			//std::cerr << " SENT: " << str << std::endl;
			std::cout << str << std::endl;
		}

		std::string recv() {
			std::string line;
			char ch = std::cin.get();
			if (ch != '\n') {
				std::getline(std::cin, line);
				line = std::string(1, ch) + line;
			}
			//std::cerr << " RECV: " << line << std::endl;
			return line;
		}
	};

}


#endif /* SSTREAM_BUFFERS_H */

