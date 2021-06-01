#pragma once

#include "catch2/catch.hpp"

#include "base26.hh"

using namespace glfdc;

class Unknowns
{
public:
  explicit Unknowns(size_t num) : unknowns(num) {}

  uintptr_t get(std::size_t i) const noexcept
  { return reinterpret_cast<const uintptr_t>(unknowns.data() + i); }

  uintptr_t get_by_name(const std::string& n) const
  {
    return get(glfdc::from_base26(n));
  }

  size_t size() const noexcept
  {
    return unknowns.size();
  }

  std::string name(uintptr_t unk) const
  {
    int* ptr = reinterpret_cast<int*>(unk);
    size_t index = ptr - unknowns.data();
    return glfdc::to_base26(index);
  }

private:
  std::vector<int> unknowns;
};

inline int unknown_value(uintptr_t p) noexcept
{
  return *reinterpret_cast<const int*>(p);
}

class UnknownGen : public Catch::Generators::IGenerator<uintptr_t>
{
  size_t current = 0;
  mutable uintptr_t current_value;
  const Unknowns &unk;

public:
  explicit UnknownGen(const Unknowns& u): unk(u) {}

  const uintptr_t& get() const override
  {
    current_value = unk.get(current);
    return current_value;
  }

  bool next() override {
    return current++ < unk.size();
  }

  std::string name() const
  {
     return unk.name(current);
  }
};

inline Catch::Generators::GeneratorWrapper<uintptr_t> unknowns(const Unknowns& n)
{
  return Catch::Generators::GeneratorWrapper<uintptr_t>(std::make_unique<UnknownGen>(n));
}

inline auto alpahabetic_unknowns()
{
  return Unknowns{26};
}
