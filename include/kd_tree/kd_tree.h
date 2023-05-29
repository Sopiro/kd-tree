#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

template <int K>
class KDTree
{
public:
    struct Point
    {
        double operator[](int idx) const
        {
            assert(idx < K);
            return coord[idx];
        }

        double& operator[](int idx)
        {
            assert(idx < K);
            return coord[idx];
        }

        double coord[K];
        void* userData;
    };

    static double dist2(const Point& p1, const Point& p2)
    {
        double d = 0.0;

        for (int i = 0; i < K; ++i)
        {
            d += (p1[i] - p2[i]) * (p1[i] - p2[i]);
        }

        return d;
    }

    struct Node
    {
        Node(const Point& p)
            : point{ p }
            , left{ nullptr }
            , right{ nullptr }
        {
        }

        Point point;
        Node* left;
        Node* right;
    };

    KDTree(std::vector<Point>& points)
        : root{ BuildTree(points, 0) }
    {
    }

    ~KDTree()
    {
        DeleteTree();
    }

    void BuildTree(std::vector<Point>& points)
    {
        if (root == nullptr)
        {
            DeleteTree();
        }

        root = BuildTree(points, 0);
    }

    void DeleteTree()
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

        root = nullptr;
    }

    Node* FindNearestNeighbor(const Point& target)
    {
        assert(root != nullptr);

        Node* nn;
        double d = DBL_MAX;
        FindNearestNeighbor(root, target, &nn, &d, 0);

        return nn;
    }

    Node* GetRootNode() const
    {
        return root;
    }

private:
    Node* BuildTree(std::vector<Point>& points, int depth = 0)
    {
        if (points.empty())
        {
            return nullptr;
        }

        int axis = depth % 2;
        size_t mid = points.size() / 2;

        std::nth_element(points.begin(), points.begin() + mid, points.end(),
                         [axis](const Point& a, const Point& b) { return a[axis] < b[axis]; });

        // Create node
        void* mem = malloc(sizeof(Node));
        Node* node = new (mem) Node(points[mid]);

        // Build left and right sub trees recursively
        std::vector<Point> leftPoints(points.begin(), points.begin() + mid);
        std::vector<Point> rightPoints(points.begin() + mid + 1, points.end());
        node->left = BuildTree(leftPoints, depth + 1);
        node->right = BuildTree(rightPoints, depth + 1);

        return node;
    }

    void FindNearestNeighbor(Node* root, const Point& target, Node** nearest, double* minDist, int depth = 0)
    {
        if (root == nullptr)
        {
            return;
        }

        double d = dist2(target, root->point);
        if (d < *minDist)
        {
            *minDist = d;
            *nearest = root;
        }

        Node* next;
        Node* other;

        // Compare axis for current depth and find next branch to descend
        int axis = depth % 2;
        if (target[axis] < root->point[axis])
        {
            next = root->left;
            other = root->right;
        }
        else
        {
            next = root->right;
            other = root->left;
        }

        // Recurse down the branch that's best according to the current depth
        FindNearestNeighbor(next, target, nearest, minDist, depth + 1);

        // We may need to check the other side of the tree.
        // If the other side is closer than the radius, then we must recurse to the other side as well.
        double border = target[axis] - root->point[axis];
        if (*minDist >= border * border)
        {
            FindNearestNeighbor(other, target, nearest, minDist, depth + 1);
        }
    }

    Node* root;
};
