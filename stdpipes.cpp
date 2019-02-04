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
 * File:   stdpipes.cpp
 * Author: Mihail Slobodyanuk <slobodyanukma@gmail.com>
 *
 * Created on February 4, 2019, 7:15 AM
 */
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include "rpc.hpp"
#include "rpc_streams.hpp"
#include "my_interface.h"

auto myrpc = rpc::make_rpc<rpc::stream_serializer, rpc::stdin_stdout_ipc_t>(
				no_args
				, one_arg
				, many_args
				, add
				, exit
				);

void client() {
	std::string const hello{"hello"};
	std::string const esc_string{"String with spaces, percents %, tab \t and new line \r\n"};
	const char* c_str = "zero terminated";
	myrpc(no_args);
	myrpc(one_arg, hello);
	myrpc(many_args, esc_string, 2, c_str, "string literal");
	std::cerr << myrpc(add, 1, 2) << std::endl;
	myrpc(exit, 0);
}

void server() {
	myrpc.listen();
}

#define PIPE_READ 0
#define PIPE_WRITE 1

int main(int argc, char* argv[]) {
	setsid();
	if (argc > 1) {
		client();
	} else {
		int nChild = 0;
		int aStdinChildPipe[2] = {0};
		int aStdoutChildPipe[2] = {0};

		if (pipe(aStdinChildPipe) < 0) {
			perror("allocating pipe for child input redirect");
			goto final;
		}
		if (pipe(aStdoutChildPipe) < 0) {
			perror("allocating pipe for child output redirect");
			goto final;
		}

		nChild = fork();
		if (0 == nChild) {
			// child continues here

			// redirect child stdin
			if (dup2(aStdinChildPipe[PIPE_READ], STDIN_FILENO) == -1) {
				goto final;
			}

			// redirect child stdout
			if (dup2(aStdoutChildPipe[PIPE_WRITE], STDOUT_FILENO) == -1) {
				goto final;
			}

			// all these are for use by parent only
			close(aStdinChildPipe[PIPE_READ]);
			close(aStdinChildPipe[PIPE_WRITE]);
			close(aStdoutChildPipe[PIPE_READ]);
			close(aStdoutChildPipe[PIPE_WRITE]);

			// run child process image
			// replace this with any exec* function find easier to use ("man exec")
			int result = execlp(argv[0], argv[0], "child", NULL);
			// if we get here at all, an error occurred, but we are in the child
			// process, so just exit
			std::cerr << "execlp error" << std::endl;
			exit(result);
		} else if (nChild > 0) {
			// parent continues here

			// redirect parent stdin
			if (dup2(aStdoutChildPipe[PIPE_READ], STDIN_FILENO) == -1) {
				goto final;
			}

			// redirect parent stdout
			if (dup2(aStdinChildPipe[PIPE_WRITE], STDOUT_FILENO) == -1) {
				goto final;
			}

			// close unused file descriptors
			close(aStdinChildPipe[PIPE_READ]);
			close(aStdinChildPipe[PIPE_WRITE]);
			close(aStdoutChildPipe[PIPE_READ]);
			close(aStdoutChildPipe[PIPE_WRITE]);

			server();
		} else {
			perror("failed to create child");
		}
		final :
		close(aStdinChildPipe[PIPE_READ]);
		close(aStdinChildPipe[PIPE_WRITE]);
		close(aStdoutChildPipe[PIPE_READ]);
		close(aStdoutChildPipe[PIPE_WRITE]);
	}
	return 0;
}

