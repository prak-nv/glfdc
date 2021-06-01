#include "bitvector.hh"
#include "expr.hh"
#include "sexpr_cmp.hh"

#include <memory>
#include <unordered_map>

namespace glfdc {

struct ExpressionBuilder
{
  ExpressionBuilder();

  Value get_binding(uintptr_t unbound); // O(log2(n))
  Value add_binding_equivalence(uintptr_t unbound, uintptr_t equivalent); // O(log2(n))

  Operand create_sexpr(OperatorKind op, Operand l, Operand r) { return create_sexpr_(op, l, r); }
  Operand create_sexpr(OperatorKind op, Operand l, Value v) { return create_sexpr_(op, l, Operand(v)); }
  Operand create_sexpr(OperatorKind op, Value v, Operand r){ return create_sexpr_(op, Operand(v), r); }
  Operand create_sexpr(OperatorKind op, Value v1, Value v2) { return create_sexpr_(op, Operand(v1), Operand(v2)); }

  std::optional<Expr> create_expr(Operand op) const noexcept
  {
    if (is_value(op))
      return std::nullopt;

    return Expr{*dag_, std::get<SExprRef>(op)};
  }

  const ExprDAG& dag() const noexcept {
    return *dag_;
  }

  auto reuses() const
  {
    return std::pair<const bitvector_t&, const bitvector_t>(reused_unbound_, reused_internal_);
  }

private:
  Value create_new_binding(uintptr_t);
  Operand create_sexpr_(OperatorKind op, Operand l, Operand r);

  void mark_reuse(SExprRef ref);

private:
  using sexpr_lookup_t = std::unordered_map<SExpr, SExprRef, sexpr_hash, sexpr_eq>;

  // TODO: something faster here:
  sexpr_lookup_t seen_exprs_;

  // only needed for lazy_eval construction
  bitvector_t reused_unbound_;
  bitvector_t reused_internal_;

  std::unique_ptr<ExprDAG> dag_;
};

} // namespace glfdc

