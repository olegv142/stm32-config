#pragma once

#define BUILD_BUG_ON(cond) extern void __build_bug_on_dummy(char a[1 - 2*!!(cond)])

#ifdef USE_FULL_ASSERT

void assert_failed(uint8_t* file, uint32_t line);

#define BUG_ON(expr) do { if (expr) assert_failed((uint8_t *)__FILE__, __LINE__); } while (0)

#else

#define BUG_ON(expr) do {} while (0)

#endif
