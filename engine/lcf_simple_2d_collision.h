#include "../base/lcf_types.h"

/* TODO redo this to use lcf_math.
   Probably requires redoing lcf math :DDD */

/* WARN min max is probably the worst way to represent rectangles in the context
   of doing AABB checks for games :(
*/

struct lcf_vec2 {
    f64 x;
    f64 y;
};

struct lcf_rect {
    lcf_vec2 min;
    lcf_vec2 max;
};


b32 test_point_rect(lcf_vec2 p, lcf_rect r) {
    return !((p.x > r.max.x)
             || (p.y > r.max.y)
             || (p.x < r.min.x)
             || (p.y < r.min.y));
}

b32 test_rect_rect(lcf_rect r1, lcf_rect r2) {
    return !((r1.min.x > r2.max.x)
             || (r1.min.y > r2.max.y)
             || (r2.min.x > r1.max.x)
             || (r2.min.y > r1.max.y));
}

struct lcf_ray {
    lcf_vec2 p;
    lcf_vec2 d;
};

void ray_rect(lcf_ray ray, lcf_rect rect) {
    
}
