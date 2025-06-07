
#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <soa/MultiArrayList.hpp>

// right now, this is just here to see if it compiles
TEST_CASE("MultiArrayList")
{
  soa::MultiArrayList<int, double, float> mal {
    {-1, -1., -1.F},
    {-2, -2., -2.F},
  };

  mal.push_back(1, 0., 0.0F);
  mal.emplace_back(2, -1., 0.1F);
  mal.push_back(3, -2., 0.2F);
  mal.push_back(4, -3., 0.3F);
  mal.push_back(5, -4., 0.4F);
  mal.push_back(6, -5., 0.5F);

  std::cout << "Full iteration:\n";
  for (auto [i, d, f] : mal) { std::cout << i << ", " << d << ", " << f << '\n'; }

  std::cout << "View int, float:\n";
  for (auto [i, f] : mal.view<int, float>()) { std::cout << i << ", " << f << '\n'; }

  std::cout << "View double:\n";
  for (auto [d] : mal.view<double>()) { std::cout << d << '\n'; }
}
