#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

struct Point
{
    double operator[](int idx) const
    {
        return idx % 2 == 0 ? x : y;
    }

    double x, y;
};

double dist2(const Point& p1, const Point& p2)
{
    double dx = p1.x - p2.x;
    double dy = p1.y - p2.y;

    return dx * dx + dy * dy;
}

struct Node
{
    Point point;
    Node* left;
    Node* right;

    Node(const Point& p)
        : point{ p }
        , left{ nullptr }
        , right{ nullptr }
    {
    }
};

Node* buildKdTree(std::vector<Point>& points, int depth = 0)
{
    if (points.empty())
    {
        return nullptr;
    }

    int axis = depth % 2;

    size_t mid = points.size() / 2;
    std::nth_element(points.begin(), points.begin() + mid, points.end(),
                     [axis](const Point& a, const Point& b) { return (axis == 0) ? a.x < b.x : a.y < b.y; });

    void* mem = malloc(sizeof(Node));
    Node* node = new (mem) Node(points[mid]);

    // Build left and right sub trees recursively
    std::vector<Point> leftPoints(points.begin(), points.begin() + mid);
    std::vector<Point> rightPoints(points.begin() + mid + 1, points.end());
    node->left = buildKdTree(leftPoints, depth + 1);
    node->right = buildKdTree(rightPoints, depth + 1);

    return node;
}

Node* closest(Node* n1, Node* n2, const Point& target)
{
    assert(n1 != nullptr || n2 != nullptr);

    if (n1 == nullptr)
    {
        return n2;
    }
    if (n2 == nullptr)
    {
        return n1;
    }

    double d1 = dist2(n1->point, target);
    double d2 = dist2(n2->point, target);

    return d1 < d2 ? n1 : n2;
}

Node* findNearestNeighbor(Node* root, const Point& target, int depth = 0)
{
    if (root == nullptr)
    {
        return nullptr;
    }

    Node* next = nullptr;
    Node* other = nullptr;

    // Compare axis for current depth and find next branch to descend
    int axis = depth % 2;
    if (axis == 0)
    {
        if (target.x < root->point.x)
        {
            next = root->left;
            other = root->right;
        }
        else
        {
            next = root->right;
            other = root->left;
        }
    }
    else
    {
        if (target.y < root->point.y)
        {
            next = root->left;
            other = root->right;
        }
        else
        {
            next = root->right;
            other = root->left;
        }
    }

    // Recurse down the branch that's best according to the current depth
    Node* nearest = findNearestNeighbor(next, target, depth + 1);
    Node* best = closest(nearest, root, target);

    double radius2 = dist2(target, best->point);

    // We may need to check the other side of the tree.
    // If the other side is closer than the radius, then we must recurse to the other side as well.
    double border;
    if (axis == 0)
    {
        border = target.x - root->point.x;
    }
    else
    {
        border = target.y - root->point.y;
    }

    if (radius2 >= border * border)
    {
        nearest = findNearestNeighbor(other, target, depth + 1);
        best = closest(nearest, best, target);
    }

    return best;
}

void deleteKdTree(Node* root)
{
    std::vector<Node*> stack;
    stack.push_back(root);

    while (!stack.empty())
    {
        Node* currentNode = stack.back();
        stack.pop_back();

        if (currentNode == nullptr)
        {
            continue;
        }

        stack.push_back(currentNode->left);
        stack.push_back(currentNode->right);

        delete currentNode;
    }
}
