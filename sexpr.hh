#pragma once

#include <cstddef>
#include <cstdint>
#include <variant>

namespace glfdc {

using scalar_type = int;

struct UnboundValue
{
  std::size_t index_;

  UnboundValue() noexcept = default;

  UnboundValue(const UnboundValue& ) noexcept = default;
  UnboundValue(UnboundValue&& ) noexcept = default;

  UnboundValue& operator=(const UnboundValue& ) noexcept = default;
  UnboundValue& operator=(UnboundValue&& ) noexcept = default;

  bool operator!=(UnboundValue other) const noexcept
  {
    return index_ != other.index_;
  }

  bool operator==(UnboundValue other) const noexcept
  {
    return index_ == other.index_;
  }
};

enum class OperatorKind : char
{
  add = '+',
  sub = '-',
  mul = '*',
  div = '/',
  mod = '%',
};

using Value = std::variant<scalar_type, UnboundValue>;

static_assert(std::is_copy_constructible_v<Value>, "");
static_assert(std::is_copy_assignable_v<Value>, "");

template <typename Tag>
struct BasicSExprRef
{
  std::size_t index_;

  BasicSExprRef() noexcept = default;

  BasicSExprRef(const BasicSExprRef&) noexcept = default;
  BasicSExprRef(BasicSExprRef&&) noexcept = default;

  BasicSExprRef& operator=(const BasicSExprRef&) noexcept = default;
  BasicSExprRef& operator=(BasicSExprRef&&) noexcept = default;

  bool operator!=(BasicSExprRef other) const noexcept
  {
    return index_ != other.index_;
  }

  bool operator==(BasicSExprRef other) const noexcept
  {
    return index_ == other.index_;
  }
};

class UnboundTag {};
class InternalTag {};

using LExprRef = BasicSExprRef<UnboundTag>;
using IExprRef = BasicSExprRef<InternalTag>;
using SExprRef = std::variant<LExprRef, IExprRef>;

inline bool is_lref(SExprRef ref) noexcept
{
  return std::get_if<LExprRef>(&ref) != nullptr;
}

inline bool is_iref(SExprRef ref) noexcept
{
  return !is_lref(ref);
}

using Operand = std::variant<std::monostate, Value, SExprRef>;

inline bool is_value(Operand op) noexcept
{
  return op.index() == 1;
}

inline bool is_unbound_value(Operand op) noexcept
{
  return is_value(op) && std::get<Value>(op).index() == 1;
}

inline bool is_scalar(Operand op) noexcept
{
  return is_value(op) && !is_unbound_value(op);
}

inline bool is_sexpr(Operand op) noexcept
{
  return op.index() == 2;
}

// Subexpression of expression reduction DAG
struct SExpr
{
  Operand lhs_;
  Operand rhs_;

  OperatorKind op_;

  SExpr() noexcept = default;
  SExpr(const SExpr&) noexcept = default;
  SExpr& operator= (const SExpr&) noexcept = default;

  bool is_unbound() const noexcept {
    return is_unbound_value(lhs_) || is_unbound_value(rhs_);
  }

  static_assert(sizeof(LExprRef) == sizeof(IExprRef), "Same sizeof of ref");
};

template <typename Tag>
inline void swap(BasicSExprRef<Tag>& l, BasicSExprRef<Tag>& r)
{
  std::swap(l.index_, r.index_);
}

inline void swap(UnboundValue& l, UnboundValue& r) noexcept
{
  std::swap(l.index_, r.index_);
}

template <typename Tag_>
inline std::size_t ref_index(BasicSExprRef<Tag_> r) noexcept 
{
  return r.index_;
}

inline std::size_t ref_index(SExprRef ref) noexcept
{
  const auto *lr = std::get_if<LExprRef>(&ref);

  if (lr)
    return ref_index(*lr);

  return std::get<IExprRef>(ref).index_;
}

} //namespace glfdc
