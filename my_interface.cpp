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
 * File:   my_interface.cpp
 * Author: Mihail Slobodyanuk <slobodyanukma@gmail.com>
 * 
 * Created on February 4, 2019, 6:51 AM
 */

#include <iostream>
#include "my_interface.h"

void no_args() {
	std::cerr << __FUNCTION__ << std::endl;
}

void one_arg(std::string const & msg) {
	std::cerr << __FUNCTION__ << " " << msg << std::endl;
}

void many_args(std::string const & msg1, int msg2, const char* c_str, const char* string_literal) {
	std::cerr << __FUNCTION__ << " " << msg1 << " " << msg2 << " " << c_str << " " << string_literal << std::endl;
}

int add(int a, int b) {
	return a + b;
}
