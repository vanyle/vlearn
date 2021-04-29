#pragma once
// everybody imports this. More or less.

#include <cstdint>
#include <cstdio>
#include <chrono>

typedef unsigned int u32;
typedef int i32;

// 32 bit friendly code if possible. 
//typedef u64 uint64_t;
//typedef i64 int64_t;

#ifndef NDEBUG
#include "vcrash.h"

#define debug(str,args...) { printf(str,## args); printf("\n");fflush(stdout); }
#define vassert(cond){ \
    if(!(cond)){ \
        fprintf(stderr,"ASSERTION FAILED in %s at line: %d\n",__FILE__,__LINE__); \
        fprintf(stderr,"%s",#cond); \
        fprintf(stderr,"\n"); \
        fflush(stderr); \
        stack_trace(false,false); \
        exit(3); \
    } \
}
#define vpanic(str,args...) { \
    fprintf(stderr,"PANIC in %s at line: %d\n",__FILE__,__LINE__); \
    fprintf(stderr,str,## args); \
    fprintf(stderr,"\n"); \
    fflush(stderr); \
    stack_trace(false,false); \
    exit(2); \
}

#else

#define debug(str,args...){}
#define vassert(cond){}
#define vpanic(str,args...){}

#endif

#include <utility> // std::forward

namespace vio{

	template <typename F, typename ... Ts>
	void time_function(const char * name,F&& f, Ts&&...args){
		std::clock_t start = std::clock();
		std::forward<F>(f)(std::forward<Ts>(args)...);
		debug("Time taken by %s: %.3f ms",name,static_cast<double>(std::clock() - start));
	}

	void utf8_console();

}
