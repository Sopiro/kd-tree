# KD Tree

Templated KD tree implementaion in C++

## Example

### Nearest Neighbor Query

```c++
#include "kd_tree.h"
#include <iostream>

int main()
{
    const int K = 2;

    using point = KDTree<K>::Point;

    std::vector<point> points = { { { 2.0, 3.0 } }, { { 5.0, 5.0 } }, { { 9.0, 6.0 } },
                                  { { 4.0, 7.0 } }, { { 8.0, 1.0 } }, { { 7.0, 2.0 } } };

    // Build the KD tree
    KDTree<K> tree(points);

    // Query point
    point target = { { 6.0, 4.0 } };

    // Perform the query
    auto r = tree.QueryNearestNeighbor(target);

    std::cout << "Nearest Neighbor: (" << r.node->point[0] << ", " << r.node->point[1] << ") "
              << "Distance: " << sqrt(r.distance2) << std::endl;

    // -> Nearest Neighbor: (5, 5) Distance: 1.41421

    return 0;
}
```

### K-Nearest Neighbor Query
```c++
int main()
{
    ...

    // Query point
    point target = { { 6.0, 4.0 } };

    int k = 3;

    // Perform the query
    auto v = tree.QueryKNearestNeighbors(target, k);

    for (int i = 0; i < k; ++i)
    {
        auto& r = v.front();

        std::cout << "Point" << k - i << ": (" << r.node->point[0] << ", " << r.node->point[1] << ") "
                  << "Distance: " << sqrt(r.distance2) << std::endl;

        std::pop_heap(v.begin(), v.end());
        v.pop_back();
    }

    // -> Point3: (4, 7) Distance: 3.60555
    // -> Point2: (7, 2) Distance: 2.23607
    // -> Point1: (5, 5) Distance: 1.41421

    return 0;
}
```

### Radius Query

```c++
int main()
{
    ...

    // Query point and radius
    point target = { { 6.0, 4.0 } };
    double radius = 3.0;

    // Define a callback object for QueryRadius function
    struct RadiusCallback
    {
        void QueryRadiusCallback(double distance2, const KDTree<2>::Node* node)
        {
            std::cout << "Point: (" << node->point[0] << ", " << node->point[1] << ") "
                      << "Distance: " << sqrt(distance2) << std::endl;
        }
    } callback;

    // Perform the query
    tree.QueryRadius(target, radius, &callback);

    // -> Point: (7, 2) Distance: 2.23607
    // -> Point: (5, 5) Distance: 1.41421

    return 0;
}
```

## Building
- Install [CMake](https://cmake.org/install/)
- Ensure CMake is in the system `PATH`
- Clone the repository `git clone https://github.com/Sopiro/kd-tree`
- Run CMake build script depend on your system
  - Visual Studio: Run `build.bat`
  - Otherwise: Run `build.sh`