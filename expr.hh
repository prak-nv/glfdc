#pragma once

#include "sexpr.hh"

#include <cassert>
#include <map>
#include <vector>

namespace glfdc {

class ExprDAG;

// NB: Expr should not outlive ExprDAG, since stores index to;
struct Expr
{
  const ExprDAG &dag_;
  SExprRef subexpr_;

  //std::string to_infix_notation() const
};

struct ExprDAG
{
  std::map<uintptr_t, std::size_t> unbound_lookup_; // XXX: any mapping - rb-tree for now
  std::vector<uintptr_t> unbound_values_;

  std::vector<SExpr> unbound_exprs_;
  std::vector<SExpr> internal_exprs_;

public:
  SExprRef add_subexpr(SExpr expr)
  {
    auto &nodes = expr.is_unbound()? unbound_exprs_ : internal_exprs_;
    size_t new_idx = nodes.size();

    nodes.push_back(expr);

    return expr.is_unbound()? SExprRef(LExprRef{new_idx}) : SExprRef(IExprRef{new_idx});
  }

  SExpr fetch(SExprRef e) const noexcept // O(1)
  {
    return (e.index() == 0)?
      fetch(std::get<LExprRef>(e)) : fetch(std::get<IExprRef>(e));
  }

  SExpr fetch(LExprRef e) const noexcept
  {
    assert(e.index_ < unbound_exprs_.size());
    return unbound_exprs_[e.index_];
  }

  SExpr fetch(IExprRef e) const noexcept
  {
    assert(e.index_ < internal_exprs_.size());
    return internal_exprs_[e.index_];
  }

  uintptr_t get_binding(UnboundValue ubv) const noexcept // O(1)
  {
    assert(ubv.index_ < unbound_values_.size());
    return unbound_values_[ubv.index_];
  }
};

} // namespace glfdc

