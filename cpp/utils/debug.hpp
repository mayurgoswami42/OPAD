#pragma once

#ifndef NDEBUG
    #include <iostream>
    #define DEBUG_LOG(x) std::cout << "DEBUG::" << x << std::endl
    // runs globally but in case of if else it puts the semicolon and hence makes it stop
    #define DEBUG_DECLARE(code) code;
    // works in conditions like if else where semicolon dont stop the flow, but can't run in class, global scope, or in struct
    #define DEBUG_STATEMENT(code) do {code;} while (0)
#else
    #define DEBUG_LOG(x) ((void)0)
    #define DEBUG_DECLARE(code)
    #define DEBUG_STATEMENT(code) ((void)0)
#endif