#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>
#include <span>
#include <vector>

// Compute complete binary tree size
inline size_t compute_size(size_t n)
{
    size_t exp = static_cast<size_t>(log2(n - 1)) + 2;
    return static_cast<size_t>(pow(2, exp));
}

template <int K, typename T = float>
class KDTree
{
public:
    struct Point
    {
        T operator[](int idx) const;
        T& operator[](int idx);

        T coord[K];
        void* userData;
    };

    struct Node
    {
        Node(const Point& p);

        Point point;
        Node* left;
        Node* right;
    };

    struct QueryResult
    {
        QueryResult(T distance2, const Node* node);
        bool operator<(const QueryResult& rhs) const;

        T distance2; // Squared distance
        const Node* node;
    };

    // Compute squared distance between two points.
    static T dist2(const Point& p1, const Point& p2);

    // Build KD tree from given points.
    KDTree(const std::span<Point>& points);
    ~KDTree();

    // Build KD tree from given points.
    // If a tree already exists, the original tree will be deleted.
    void BuildTree(const std::span<Point>& points);

    // Delete internal KD tree.
    void DeleteTree();

    // Query functions.

    // Returns the nearest neighbor data.
    QueryResult QueryNearestNeighbor(const Point& target);

    // Returns the max-heap of K nearest neighbors results.
    std::vector<QueryResult> QueryKNearestNeighbors(const Point& target, int k);

    // Callback object should implement the QueryRadiusCallback(T distance2, const Node* node) function.
    template <typename F>
    void QueryRadius(const Point& target, T radius, F* callback);

    // Returns the internal tree object.
    const Node* GetRootNode() const;

private:
    Node* BuildTree(const std::span<Point>& points, int* indices, int count, int depth);

    void QueryNearestNeighbor(Node* root, const Point& target, Node** nearest, T* minDist, int depth);
    void QueryKNearestNeighbors(Node* root, const Point& target, int k, std::vector<QueryResult>& pq, int depth);
    template <typename F>
    void QueryRadius(Node* root, const Point& target, T radius2, F* callback, int depth);

    Node* root;
    std::vector<Node> nodes;

#if _DEBUG
    int nodeCount = 0;
#endif
};

// Implementations

template <int K, typename T>
inline T KDTree<K, T>::Point::operator[](int idx) const
{
    assert(idx < K);
    return coord[idx];
}

template <int K, typename T>
inline T& KDTree<K, T>::Point::operator[](int idx)
{
    assert(idx < K);
    return coord[idx];
}

template <int K, typename T>
inline KDTree<K, T>::Node::Node(const Point& p)
    : point{ p }
    , left{ nullptr }
    , right{ nullptr }
{
}

template <int K, typename T>
inline KDTree<K, T>::QueryResult::QueryResult(T distance2, const Node* node)
    : distance2{ distance2 }
    , node{ node }
{
}

template <int K, typename T>
inline bool KDTree<K, T>::QueryResult::operator<(const QueryResult& rhs) const
{
    return distance2 < rhs.distance2;
}

template <int K, typename T>
inline T KDTree<K, T>::dist2(const Point& p1, const Point& p2)
{
    T d = 0;

    for (int i = 0; i < K; ++i)
    {
        d += (p1[i] - p2[i]) * (p1[i] - p2[i]);
    }

    return d;
}

template <int K, typename T>
inline KDTree<K, T>::KDTree(const std::span<Point>& points)
{
    BuildTree(points);
}

template <int K, typename T>
inline KDTree<K, T>::~KDTree()
{
    DeleteTree();
}

template <int K, typename T>
inline void KDTree<K, T>::BuildTree(const std::span<Point>& points)
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

template <int K, typename T>
inline void KDTree<K, T>::DeleteTree()
{
    nodes.clear();
    root = nullptr;
}

template <int K, typename T>
inline typename KDTree<K, T>::QueryResult KDTree<K, T>::QueryNearestNeighbor(const Point& target)
{
    assert(root != nullptr);

    Node* nn;
    T d = std::numeric_limits<T>::max();
    QueryNearestNeighbor(root, target, &nn, &d, 0);

    return QueryResult{ d, nn };
}

template <int K, typename T>
inline std::vector<typename KDTree<K, T>::QueryResult> KDTree<K, T>::QueryKNearestNeighbors(const Point& target, int k)
{
    assert(root != nullptr);

    // Priority queue
    std::vector<QueryResult> pq;
    pq.reserve(k + 1);

    QueryKNearestNeighbors(root, target, k, pq, 0);

    return pq;
}

template <int K, typename T>
template <typename F>
inline void KDTree<K, T>::QueryRadius(const Point& target, T radius, F* callback)
{
    assert(root != nullptr);

    QueryRadius(root, target, radius * radius, callback, 0);
}

template <int K, typename T>
inline const typename KDTree<K, T>::Node* KDTree<K, T>::GetRootNode() const
{
    return root;
}

template <int K, typename T>
inline typename KDTree<K, T>::Node* KDTree<K, T>::BuildTree(const std::span<Point>& points, int* indices, int count, int depth)
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

template <int K, typename T>
inline void KDTree<K, T>::QueryNearestNeighbor(Node* node, const Point& target, Node** nearest, T* minDist, int depth)
{
    if (node == nullptr)
    {
        return;
    }

    T d = dist2(target, node->point);
    if (d < *minDist)
    {
        *minDist = d;
        *nearest = node;
    }

    Node* next;
    Node* other;

    // Compare axis for current depth and find next branch to descend
    int axis = depth % K;
    if (target[axis] < node->point[axis])
    {
        next = node->left;
        other = node->right;
    }
    else
    {
        next = node->right;
        other = node->left;
    }

    // Recurse down the branch that's best according to the current depth
    QueryNearestNeighbor(next, target, nearest, minDist, depth + 1);

    // We may need to check the other side of the tree
    // If the other side is closer than the radius, then we must recurse to the other side as well
    T border = target[axis] - node->point[axis];
    if (*minDist > border * border)
    {
        QueryNearestNeighbor(other, target, nearest, minDist, depth + 1);
    }
}

template <int K, typename T>
inline void KDTree<K, T>::QueryKNearestNeighbors(Node* node, const Point& target, int k, std::vector<QueryResult>& pq, int depth)
{
    if (node == nullptr)
    {
        return;
    }

    T d = dist2(target, node->point);
    if (pq.size() < k || d < pq.front().distance2)
    {
        pq.emplace_back(d, node);
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
    if (target[axis] < node->point[axis])
    {
        next = node->left;
        other = node->right;
    }
    else
    {
        next = node->right;
        other = node->left;
    }

    QueryKNearestNeighbors(next, target, k, pq, depth + 1);

    T border = target[axis] - node->point[axis];
    if (pq.size() < k || border * border < pq.front().distance2)
    {
        QueryKNearestNeighbors(other, target, k, pq, depth + 1);
    }
}

template <int K, typename T>
template <typename F>
inline void KDTree<K, T>::QueryRadius(Node* node, const Point& target, T radius2, F* callback, int depth)
{
    if (node == nullptr)
    {
        return;
    }

    T d = dist2(target, node->point);
    if (d < radius2)
    {
        callback->QueryRadiusCallback(d, static_cast<const Node*>(node));
    }

    Node* next;
    Node* other;

    // Compare axis for current depth and find next branch to descend
    int axis = depth % K;
    if (target[axis] < node->point[axis])
    {
        next = node->left;
        other = node->right;
    }
    else
    {
        next = node->right;
        other = node->left;
    }

    QueryRadius(next, target, radius2, callback, depth + 1);

    T border = target[axis] - node->point[axis];
    if (radius2 > border * border)
    {
        QueryRadius(other, target, radius2, callback, depth + 1);
    }
}