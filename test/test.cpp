#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "kd_tree/kd_tree.h"
#include "timer.h"

#include <random>
#include <vector>

inline std::minstd_rand prng;

inline void Srand(unsigned int seed)
{
    prng.seed(seed);
}

inline double Prand()
{
    return double(prng()) / std::minstd_rand::max();
}

inline double Prand(double min, double max)
{
    return min + (max - min) * Prand();
}

TEST_CASE("Memory leak check")
{
#if defined(_WIN32) && defined(_DEBUG)
    // Enable memory-leak reports
    _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG));
#endif
}

TEST_CASE("Nearest neighbor query")
{
    int count = 1000000;

    using point = KDTree<2>::Point;
    using node = KDTree<2>::Node;

    std::vector<point> points(count);

    for (int i = 0; i < count; ++i)
    {
        points[i][0] = Prand(-10000, 10000);
        points[i][1] = Prand(-10000, 10000);
    }

    // Target point
    // Srand(unsigned int(time(0)));
    point target{ Prand(-10000, 10000), Prand(-10000, 10000) };

    Timer timer;

    // Build KD-tree
    KDTree<2> tree(points);

    timer.Mark();

    // Brute force
    double bd = DBL_MAX;
    point bp = target;
    for (int i = 0; i < points.size(); ++i)
    {
        double c = tree.dist2(target, points[i]);

        if (c < bd)
        {
            bd = c;
            bp = points[i];
        }
    }

    timer.Mark();

    // Nearest neighbor
    const node* nn = tree.QueryNearestNeighbor(target).node;
    const point& np = nn->point;

    timer.Mark();

    // Print
    std::cout << "\n----------------------\n" << std::endl;
    std::cout << "Nearest neighbor query" << std::endl;
    std::cout << "Target point\t: (" << target[0] << ", " << target[1] << ")" << std::endl;
    std::cout << std::endl;
    std::cout << "NN by tree\t: (" << np[0] << ", " << np[1] << ")" << std::endl;
    std::cout << "NN by BF\t: (" << bp[0] << ", " << bp[1] << ")" << std::endl;
    std::cout << std::endl;
    std::cout << "Number of points: " << count << std::endl;
    std::cout << "Kd-tree build\t: " << timer.Get() * 1000 << "ms" << std::endl;
    std::cout << "BF query\t: " << timer.Get() * 1000 << "ms" << std::endl;
    std::cout << "Kd-tree query\t: " << timer.Get() * 1000 << "ms" << std::endl;

    REQUIRE_EQ(bp[0], np[0]);
    REQUIRE_EQ(bp[1], np[1]);
}

TEST_CASE("Radius query")
{
    int count = 1000000;

    using point = KDTree<2>::Point;
    using node = KDTree<2>::Node;

    std::vector<point> points(count);

    for (int i = 0; i < count; ++i)
    {
        points[i][0] = Prand(-10000, 10000);
        points[i][1] = Prand(-10000, 10000);
    }

    // Build KD-tree
    KDTree<2> tree(points);

    point target{ Prand(-10000, 10000), Prand(-10000, 10000) };
    double radius = 100.0;

    struct TempCallback
    {
        void QueryRadiusCallback(double distance2, const node* node)
        {
            double distance = sqrt(distance2);
            REQUIRE_EQ(distance < r, true);
            ++count;
        }

        double r;
        int count;
    } callback;

    callback.r = radius;
    callback.count = 0;

    Timer timer;

    // Brute force
    int bfCount = 0;
    for (int i = 0; i < count; ++i)
    {
        if (tree.dist2(target, points[i]) < radius * radius)
        {
            ++bfCount;
        }
    }

    timer.Mark();

    tree.QueryRadius(target, radius, &callback);

    timer.Mark();

    REQUIRE_EQ(callback.count, bfCount);
    std::cout << "\n----------------------\n" << std::endl;
    std::cout << "Radius query" << std::endl;
    std::cout << "Number of points: " << count << std::endl;
    std::cout << "BF query\t: " << timer.Get() * 1000 << "ms" << std::endl;
    std::cout << "Kd-tree query\t: " << timer.Get() * 1000 << "ms" << std::endl;
}

TEST_CASE("K-Nearest neighbor query")
{
    int count = 1000000;

    using point = KDTree<2>::Point;
    using node = KDTree<2>::Node;

    std::vector<point> points(count);

    for (int i = 0; i < count; ++i)
    {
        points[i][0] = Prand(-10000, 10000);
        points[i][1] = Prand(-10000, 10000);
    }

    // Build KD-tree
    KDTree<2> tree(points);

    point target{ Prand(-10000, 10000), Prand(-10000, 10000) };

    std::cout << "\n----------------------\n" << std::endl;
    std::cout << "K-Nearest neighbor query" << std::endl;

    Timer timer;

    int k = 10;
    auto v = tree.QueryKNearestNeighbors(target, k);

    timer.Mark();

    // It returns max heap
    REQUIRE_EQ(std::is_heap(v.begin(), v.end()), true);
    REQUIRE_EQ(v.size(), k);

    std::cout << "Number of points: " << count << std::endl;
    std::cout << "Kd-tree query\t: " << timer.Get() * 1000 << "ms" << std::endl;

    for (int i = 0; i < k; ++i)
    {
        std::cout << sqrt(v.front().distance2) << std::endl;

        std::pop_heap(v.begin(), v.end());
        v.pop_back();
    }
}