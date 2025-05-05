#pragma once

#include "defines.hpp"

struct vec2 {
	union {
		struct {f32 x, y;};
		struct {f32 u, v;};
		f32 elements[2];
	};
};

struct vec3 {
	union {
		struct { f32 x, y, z;};
		struct { f32 r, g, b;};
		struct { f32 u, v, w;};
		f32 elements[3];
	};
};

struct vec4 {
	union {
		struct { f32 x, y, z, w;};
		struct { f32 r, g, b, a;};
		f32 elements[4];
	};
};
