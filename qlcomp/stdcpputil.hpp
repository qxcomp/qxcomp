#ifndef STDCPPUTIL_HPP
#define STDCPPUTIL_HPP

#include <algorithm>
#include <cstddef>
#include <cstdlib>

// Fisher–Yates 就地随机洗牌：打乱 [first, last)。
// 纯 std、无 Qt 依赖；调用方自行 srand() 播种（沿用项目既有 rand 用法）。
// 兼容 C++11 与 C++17（替代 C++17 已移除的 std::random_shuffle）。
template <class RandomIt>
void randomShuffle(RandomIt first, RandomIt last)
{
    const std::ptrdiff_t n = last - first;
    for (std::ptrdiff_t i = n - 1; i > 0; --i) {
        const std::ptrdiff_t j = (std::ptrdiff_t)(std::rand() % (i + 1));
        std::iter_swap(first + i, first + j);
    }
}

#endif
