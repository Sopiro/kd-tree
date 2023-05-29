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
    const node* nn = tree.FindNearestNeighbor(target);
    const point& np = nn->point;

    timer.Mark();

    // Print
    std::cout << "Target point\t: (" << target[0] << ", " << target[1] << ")" << std::endl;
    std::cout << std::endl;
    std::cout << "NN by tree\t: (" << np[0] << ", " << np[1] << ")" << std::endl;
    std::cout << "NN by BF\t: (" << bp[0] << ", " << bp[1] << ")" << std::endl;
    std::cout << std::endl;
    std::cout << "Number of points: " << count << std::endl;
    std::cout << "Kd-tree build\t: " << timer.Get() * 1000 << "ms" << std::endl;
    std::cout << "BF search\t: " << timer.Get() * 1000 << "ms" << std::endl;
    std::cout << "Kd-tree search\t: " << timer.Get() * 1000 << "ms" << std::endl;

    REQUIRE_EQ(bp[0], np[0]);
    REQUIRE_EQ(bp[1], np[1]);
}