#pragma once

#include "sexpr.hh"

#include <cassert>
#include <optional>

namespace glfdc {

inline scalar_type cfold(OperatorKind op, scalar_type l, scalar_type r) noexcept
{
  switch(op)
  {
  case OperatorKind::add:
    return l + r; 
  case OperatorKind::sub:
    return l - r;
  case OperatorKind::mul:
    return l * r;
  case OperatorKind::div:
  {
    if (r == 0) return 0;
    return l / r;
  }
  case OperatorKind::mod:
  {
    if (r == 0) return 0;
    return l % r;
  }
  }

  assert(false && "Unreachable");
}

inline std::optional<scalar_type> cfold(OperatorKind op, Operand v1, Operand v2)
{
  auto as_scalar = [] (Operand op) -> scalar_type {
    return std::get<scalar_type>(std::get<Value>(op));
  };

  if (!is_value(v1) || is_unbound_value(v1))
    return std::nullopt;

  if (!is_value(v2) || is_unbound_value(v2))
    return std::nullopt;

  return cfold(op, as_scalar(v1), as_scalar(v2));
}

} // namespace glfdc

