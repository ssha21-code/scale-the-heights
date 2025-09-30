// CollisionRects.h
#pragma once
#include <raylib.h>
#include <array>
#include <cmath>

namespace Collision {

// Helper: Check if point P is inside triangle ABC
inline bool PointInTriangle(const Vector2 &P, const Vector2 &A, const Vector2 &B, const Vector2 &C) {
    float s = A.y * C.x - A.x * C.y + (C.y - A.y) * P.x + (A.x - C.x) * P.y;
    float t = A.x * B.y - A.y * B.x + (A.y - B.y) * P.x + (B.x - A.x) * P.y;

    if ((s < 0) != (t < 0)) return false;

    float area = -B.y * C.x + A.y * (C.x - B.x) + A.x * (B.y - C.y) + B.x * C.y;
    return area < 0 ? (s <= 0 && s + t >= area) : (s >= 0 && s + t <= area);
}

// Check collision between two rectangles represented by 4 corners each
inline bool CheckCollisionRectCorners(const std::array<Vector2, 4> &rect1, const std::array<Vector2, 4> &rect2) {
    // Split each rectangle into two triangles
    std::array<std::array<Vector2, 3>, 2> tris1 = {{{rect1[0], rect1[1], rect1[2]}, {rect1[0], rect1[2], rect1[3]}}};
    std::array<std::array<Vector2, 3>, 2> tris2 = {{{rect2[0], rect2[1], rect2[2]}, {rect2[0], rect2[2], rect2[3]}}};

    // Check all triangle pairs
    for (auto &t1 : tris1) {
        for (auto &t2 : tris2) {
            // Check if any vertex of t1 is inside t2
            for (const auto &v : t1) {
                if (PointInTriangle(v, t2[0], t2[1], t2[2])) return true;
            }
            // Check if any vertex of t2 is inside t1
            for (const auto &v : t2) {
                if (PointInTriangle(v, t1[0], t1[1], t1[2])) return true;
            }
        }
    }

    return false;
}

bool ResolveCollision(Vector2 &entityPos, const Vector2 &entitySize,
                      Vector2 &entityVelocity, const Rectangle &solid,
                      bool &isOnPlatform) {
    Rectangle entityRect = {entityPos.x, entityPos.y, entitySize.x, entitySize.y};

    if (CheckCollisionRecs(entityRect, solid)) {
        // Landing on top
        if (entityVelocity.y >= 0 &&
            entityPos.y + entitySize.y <= solid.y + entityVelocity.y) {
            entityPos.y = solid.y - entitySize.y;
            entityVelocity.y = 0;
            isOnPlatform = true;
        }
        // Hitting the underside
        else if (entityVelocity.y < 0 &&
                 entityPos.y >= solid.y + solid.height + entityVelocity.y) {
            entityPos.y = solid.y + solid.height;
            entityVelocity.y = 0;
        }
        // Side collisions (only when not already standing on top/bottom)
        else {
            float overlapLeft   = (entityPos.x + entitySize.x) - solid.x;
            float overlapRight  = (solid.x + solid.width) - entityPos.x;
            float overlapTop    = (entityPos.y + entitySize.y) - solid.y;
            float overlapBottom = (solid.y + solid.height) - entityPos.y;

            float minOverlap = fmin(fmin(overlapLeft, overlapRight),
                                    fmin(overlapTop, overlapBottom));

            if (minOverlap == overlapLeft) {
                entityPos.x = solid.x - entitySize.x;
            } else if (minOverlap == overlapRight) {
                entityPos.x = solid.x + solid.width;
            } else if (minOverlap == overlapTop) {
                entityPos.y = solid.y - entitySize.y;
                entityVelocity.y = 0;
                isOnPlatform = true;
            } else {
                entityPos.y = solid.y + solid.height;
                entityVelocity.y = 0;
            }
        }
        return true; // collision happened
    }

    return false; // no collision
}

} // namespace Collision

namespace Random {
float GetRandomFloat(float min, float max) {
    return min + (float)GetRandomValue(0, 10000) / 10000.0f * (max - min);
}
}
