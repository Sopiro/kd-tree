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

TEST_CASE("Test")
{
    int count = 1000000;

    std::vector<Point> points(count);

    for (int i = 0; i < count; ++i)
    {
        points.emplace_back(Prand(-10000, 10000), Prand(-10000, 10000));
    }

    // Target point
    // Srand(unsigned int(time(0)));
    Point target = { Prand(-10000, 10000), Prand(-10000, 10000) };

    Timer timer;

    // Build KD-tree
    Node* root = buildKdTree(points);
    timer.Mark();

    // Brute force
    double cd = DBL_MAX;
    Point cp = target;
    for (int i = 0; i < points.size(); ++i)
    {
        double c = dist2(target, points[i]);

        if (c < cd)
        {
            cd = c;
            cp = points[i];
        }
    }

    timer.Mark();

    // Nearest neighbor
    Node* nn = findNearestNeighbor(root, target);

    timer.Mark();

    // Print
    std::cout << "Target point\t: (" << target.x << ", " << target.y << ")" << std::endl;
    std::cout << std::endl;
    std::cout << "NN by tree\t: (" << nn->point.x << ", " << nn->point.y << ")" << std::endl;
    std::cout << "NN by BF\t: (" << cp.x << ", " << cp.y << ")" << std::endl;
    std::cout << std::endl;
    std::cout << "Points count\t: " << count << std::endl;
    std::cout << "Kd-tree build\t: " << timer.Get() * 1000 << "ms" << std::endl;
    std::cout << "BF search\t: " << timer.Get() * 1000 << "ms" << std::endl;
    std::cout << "Kd-tree search\t: " << timer.Get() * 1000 << "ms" << std::endl;

    REQUIRE_EQ(cp.x, nn->point.x);
    REQUIRE_EQ(cp.y, nn->point.y);

    deleteKdTree(root);
}