
// Source: https://www.scs.stanford.edu/~dm/blog/va-opt.html

#pragma once

#include "manipulation.h"

#define EXPAND(...) EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__))))
#define EXPAND4(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) __VA_ARGS__

#define FOR_EACH(macro, ...)                                    \
__VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define FOR_EACH_HELPER(macro, a1, ...)                         \
macro(a1)                                                     \
__VA_OPT__(FOR_EACH_AGAIN PARENS (macro, __VA_ARGS__))
#define FOR_EACH_AGAIN() FOR_EACH_HELPER

#define FOR_EACH_W_ARG(macro,arg, ...)                                    \
__VA_OPT__(EXPAND(FOR_EACH_W_ARG_HELPER(macro,arg, __VA_ARGS__)))
#define FOR_EACH_W_ARG_HELPER(macro,arg, a1, ...)                         \
macro(arg,a1)                                                     \
__VA_OPT__(FOR_EACH_W_ARG_AGAIN PARENS (macro, arg, __VA_ARGS__))
#define FOR_EACH_W_ARG_AGAIN() FOR_EACH_W_ARG_HELPER

#define COUNTER_FOR_EACH(macro, counter, ...)                      \
__VA_OPT__(EXPAND(COUNTER_FOR_EACH_HELPER(macro, counter, __VA_ARGS__)))
#define COUNTER_FOR_EACH_HELPER(macro, counter, a1, ...)                    \
macro(a1, counter)                                                     \
__VA_OPT__(COUNTER_FOR_EACH_AGAIN PARENS (macro, counter + 1, __VA_ARGS__))
#define COUNTER_FOR_EACH_AGAIN() COUNTER_FOR_EACH_HELPER

#define COUNTER_FOR_EACH(macro, counter, ...)                      \
__VA_OPT__(EXPAND(COUNTER_FOR_EACH_HELPER(macro, counter, __VA_ARGS__)))
#define COUNTER_FOR_EACH_HELPER(macro, counter, a1, ...)                    \
macro(a1, counter)                                                     \
__VA_OPT__(COUNTER_FOR_EACH_AGAIN PARENS (macro, counter + 1, __VA_ARGS__))
#define COUNTER_FOR_EACH_AGAIN() COUNTER_FOR_EACH_HELPER

#define COUNTER_ARG_FOR_EACH(macro, arg, counter, ...)                      \
__VA_OPT__(EXPAND(COUNTER_ARG_FOR_EACH_HELPER(macro, arg, counter, __VA_ARGS__)))
#define COUNTER_ARG_FOR_EACH_HELPER(macro, arg, counter, a1, ...)                    \
macro(arg, a1, counter)                                                     \
__VA_OPT__(COUNTER_ARG_FOR_EACH_AGAIN PARENS (macro, arg, counter + 1, __VA_ARGS__))
#define COUNTER_ARG_FOR_EACH_AGAIN() COUNTER_ARG_FOR_EACH_HELPER

#define FOR_EACH_FORWARD(macro, ...)                                    \
__VA_OPT__(EXPAND(FOR_EACH_FORWARD_HELPER(macro, __VA_ARGS__)))
#define FOR_EACH_FORWARD_HELPER(macro, a1, ...)                         \
macro(a1,__VA_ARGS__)                                                     \
__VA_OPT__(FOR_EACH_FORWARD_AGAIN PARENS (macro, __VA_ARGS__))
#define FOR_EACH_FORWARD_AGAIN() FOR_EACH_FORWARD_HELPER

#define FOR_EACH_FORWARD_W_ARG(macro, arg, ...)                                    \
__VA_OPT__(EXPAND(FOR_EACH_FORWARD_W_ARG_HELPER(macro, arg, __VA_ARGS__)))
#define FOR_EACH_FORWARD_W_ARG_HELPER(macro, arg, a1, ...)                         \
macro(arg,a1,__VA_ARGS__)                                                     \
__VA_OPT__(FOR_EACH_FORWARD_W_ARG_AGAIN PARENS (macro, arg, __VA_ARGS__))
#define FOR_EACH_FORWARD_W_ARG_AGAIN() FOR_EACH_FORWARD_W_ARG_HELPER

