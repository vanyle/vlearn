/*
 * utils.cpp
 *
 *  Created on: 19 avr. 2021
 *      Author: vanyle
 */
#include "utils.h"

#ifdef WIN32
#include <windows.h>
#endif

namespace vio{

void utf8_console(){
#ifdef WIN32
	SetConsoleOutputCP(65001);
#endif
// else do nothing, this is a windows only thing.
}

}
