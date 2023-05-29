#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>
#include <vector>

inline size_t compute_size(size_t n)
{
    size_t exp = static_cast<size_t>(log2(n - 1)) + 2;
    return static_cast<size_t>(pow(2, exp));
}

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

    struct QueryResult
    {
        bool operator<(const QueryResult& rhs)
        {
            return distance2 < rhs.distance2;
        }

        double distance2;
        const Node* node;
    };

    KDTree(const std::vector<Point>& points)
    {
        BuildTree(points);
    }

    ~KDTree()
    {
        DeleteTree();
    }

    void BuildTree(const std::vector<Point>& points)
    {
        if (root != nullptr)
        {
            DeleteTree();
        }

        // Reserve nodes
        size_t bufferSize = compute_size(points.size());
        nodes.reserve(bufferSize);

        // Build tree with points indices vector to preserve original data
        std::vector<int> indices(points.size());
        std::iota(indices.begin(), indices.end(), 0);

        root = BuildTree(points, indices.data(), (int)points.size(), 0);
    }

    void DeleteTree()
    {
        nodes.clear();
        root = nullptr;
    }

    QueryResult QueryNearestNeighbor(const Point& target)
    {
        assert(root != nullptr);

        Node* nn;
        double d = DBL_MAX;
        QueryNearestNeighbor(root, target, &nn, &d, 0);

        return QueryResult{ d, nn };
    }

    std::vector<QueryResult> QueryKNearestNeighbors(const Point& target, int k)
    {
        assert(root != nullptr);

        // Priority queue
        std::vector<QueryResult> pq;
        pq.reserve(k + 1);

        QueryKNearestNeighbors(root, target, k, pq, 0);

        return pq;
    }

    template <typename T>
    void QueryRadius(const Point& target, double radius, T* callback)
    {
        assert(root != nullptr);

        QueryRadius(root, target, radius * radius, callback, 0);
    }

    const Node* GetRootNode() const
    {
        return root;
    }

private:
    Node* BuildTree(const std::vector<Point>& points, int* indices, int count, int depth)
    {
        if (count <= 0)
        {
            return nullptr;
        }

        int axis = depth % K;
        int mid = count / 2;

        std::nth_element(indices, indices + mid, indices + count,
                         [&](int left, int right) { return points[left][axis] < points[right][axis]; });

        // Create kd tree node
        Node& node = nodes.emplace_back(points[indices[mid]]);

#if _DEBUG
        ++nodeCount;
        assert(nodeCount < nodes.capacity());
#endif

        // Build left and right sub trees recursively
        node.left = BuildTree(points, indices, mid, depth + 1);
        node.right = BuildTree(points, indices + mid + 1, count - mid - 1, depth + 1);

        return &node;
    }

    void QueryNearestNeighbor(Node* root, const Point& target, Node** nearest, double* minDist, int depth)
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
        int axis = depth % K;
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
        QueryNearestNeighbor(next, target, nearest, minDist, depth + 1);

        // We may need to check the other side of the tree.
        // If the other side is closer than the radius, then we must recurse to the other side as well.
        double border = target[axis] - root->point[axis];
        if (*minDist > border * border)
        {
            QueryNearestNeighbor(other, target, nearest, minDist, depth + 1);
        }
    }

    void QueryKNearestNeighbors(Node* root, const Point& target, int k, std::vector<QueryResult>& pq, int depth)
    {
        if (root == nullptr)
        {
            return;
        }

        double d = dist2(target, root->point);
        if (pq.size() < k || d < pq.front().distance2)
        {
            pq.emplace_back(d, root);
            std::push_heap(pq.begin(), pq.end());

            if (pq.size() > k)
            {
                std::pop_heap(pq.begin(), pq.end());
                pq.pop_back();
            }
        }

        Node* next;
        Node* other;

        int axis = depth % K;
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

        QueryKNearestNeighbors(next, target, k, pq, depth + 1);

        double border = target[axis] - root->point[axis];
        if (pq.size() < k || border * border < pq.front().distance2)
        {
            QueryKNearestNeighbors(other, target, k, pq, depth + 1);
        }
    }

    template <typename T>
    void QueryRadius(Node* root, const Point& target, double radius2, T* callback, int depth)
    {
        if (root == nullptr)
        {
            return;
        }

        double d = dist2(target, root->point);
        if (d < radius2)
        {
            callback->QueryRadiusCallback(d, static_cast<const Node*>(root));
        }

        Node* next;
        Node* other;

        // Compare axis for current depth and find next branch to descend
        int axis = depth % K;
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

        QueryRadius(next, target, radius2, callback, depth + 1);

        double border = target[axis] - root->point[axis];
        if (radius2 > border * border)
        {
            QueryRadius(other, target, radius2, callback, depth + 1);
        }
    }

    Node* root;
    std::vector<Node> nodes;

#if _DEBUG
    int nodeCount = 0;
#endif
};
