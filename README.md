# KD Tree

Templated KD tree implementaion in C++

## Example

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

    auto result = tree.QueryNearestNeighbor(target);

    std::cout << "Nearest Neighbor: (" << result.node->point[0] << ", " << result.node->point[1] << ")" << std::endl;
    std::cout << "Distance: " << sqrt(result.distance2) << std::endl;
    // -> Nearest Neighbor: (5, 5)
    // -> Distance: 1.41421

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