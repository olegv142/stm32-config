#pragma once

#define BUILD_BUG_ON(cond) extern void __build_bug_on_dummy(char a[1 - 2*!!(cond)])

#ifdef USE_FULL_ASSERT

void assertion_failed(const char* file, unsigned line);

#define BUG_ON(expr) do { if (expr) assertion_failed(__FILE__, __LINE__); } while (0)

#else

#define BUG_ON(expr) do {} while (0)

#endif
