#pragma once

// Use if instead?
#if defined( __INTELLISENSE__ ) || defined( __RESHARPER__ )
#define IN_EDITOR 1
#endif // __INTELLISENSE__ || __RESHARPER__

#define EMPTY( ... )

#define VARGS_(_10, _9, _8, _7, _6, _5, _4, _3, _2, _1, N, ...) N
#define VARGS(...) VARGS_( __VA_ARGS__ __VA_OPT__( , ) 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define FORWARD( ... ) __VA_ARGS__

#define CONCAT_(a, b) a ## b
#define CONCAT(a, b) CONCAT_(a, b)

#define FIRST( A, ... ) A
#define AFTER_FIRST( A, ... ) __VA_ARGS__
#define SECOND( A, B, ... ) B

#define PARENS ()

#define IS_PACK( ... ) 1,

#define UNWRAP_PACK( ... ) __VA_ARGS__
#define UNWRAP( Pack ) UNWRAP_ ## Pack

#if defined( IN_EDITOR )
// Selects B in editor and A in runtime
#define PLACEHOLDER( A, B ) B
#define COMMENT( ... ) static_assert( true, #__VA_ARGS__ ); /* Will only exist inside the editor. */ \
    
#else // IN_EDITOR
// Selects B in editor and A in runtime
#define PLACEHOLDER( A, B ) A
#define COMMENT( ... )
#endif // !IN_EDITOR
