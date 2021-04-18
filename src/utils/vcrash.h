/**
	@notitle
	VCrash is a windows only library used for debugging. (for MinGW64)
	It displays a stack trace on crashs.

	Include it once, call setup_crash_handler() and you're good to go!
	Also, link against dbghelp

	Also, if you want to print the stack at some point, just call:
	stack_trace(false,false);

	To make full use of this library, don't forget to compile with the -g option in include debugging symbols in your build.
	Moreover, generate a pdb file for your executable with cv2pdb for full debugging information.
	Without pdb file, I'm unable to retreive information like function names, or line numbers from the executable.

	Also, the line numbers are approximative and might not take header files / macros into account.

	Usage:
	@code
		void setup_crash_handler(); // call this at the start of your thread

		// ... do stuff
		stack_trace(false,false); // do this to print a stack trace, always put false as argument.
	@endcode
*/
#pragma once

#include <string.h>

#include <windows.h>
#include <DbgHelp.h>

#include <csignal>
#include <iostream>
#include <stdio.h>
#include <cstdlib>


// on crash mode, don't print calls made after the crash occured
// on cut setup, don't print calls made before main is called.
void stack_trace(bool crashMode, bool cutSetup);

void on_process_crash(int sig);
void setup_crash_handler();
