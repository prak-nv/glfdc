#pragma once

#include "bitvector.hh"
#include "sexpr.hh"
#include "stack.hh"
#include "sparse_map.hh"

#include <algorithm>
#include <functional>
#include <optional>

#include <iostream>
namespace glfdc {

class Expr;
class ExprDAG;

struct EvalState;

struct Operation 
{
  const SExprRef ref;

  // TODO: bitfields/ushort? 12b/16b should be plenty and we could pack it more
  unsigned nsubops; // number of following operations to skip if lazy evaluated
  unsigned noperands; // total number of operands on stack to calculate subexpression value

  static constexpr unsigned INVALID_POP = unsigned(-1);
};

using opt_index_t = std::optional<std::size_t>;

struct ReusedExprMapping
{
  const sparse_map slot_mapping;
  const size_t unbound_count;

  opt_index_t slot(SExprRef ref) const noexcept
  {
    const auto *lexpr = std::get_if<LExprRef>(&ref);

    if (lexpr)
      return slot_mapping.find(lexpr->index_);

    // addend of reused unbound
    return slot_mapping.find(std::get<IExprRef>(ref).index_ + unbound_count);
  }

  std::size_t size() const noexcept
  {
    return slot_mapping.size();
  }

  bool empty() const noexcept
  {
    return slot_mapping.empty();
  }

  // named constructors
  static ReusedExprMapping create_eager_mapping()
  {
    return {sparse_map{}, std::size_t(0)};
  }

  static ReusedExprMapping create_lazy_mapping(const bitvector_t& reused_unbound_sexprs,
                                               const bitvector_t& reused_inner_sexprs) // O(n+k)
  {
    sparse_map mapping(reused_unbound_sexprs.size() + reused_inner_sexprs.size());

    std::size_t slot_counter = 0;

    for (std::size_t i=0; i<reused_unbound_sexprs.size(); ++i)
    {
      if (reused_unbound_sexprs[i])
        mapping.insert(i, slot_counter++);
    }

    // Map with addend of number of unbound sexprs
    const std::size_t addend = reused_unbound_sexprs.size();

    for (std::size_t i=0; i<reused_inner_sexprs.size(); ++i)
    {
      if (reused_inner_sexprs[i])
        mapping.insert(i + addend, slot_counter++);
    }

    return {mapping, addend};
  }
};

struct EvalState
{
  using LazyScalar = std::optional<scalar_type>;
  using LazyScalarSlot = LazyScalar*;

  explicit EvalState(const ReusedExprMapping& mapping): memo_(mapping.size()), mapping_(mapping)
  {
  }

  EvalState(const EvalState&) = default;

  LazyScalarSlot load(SExprRef sexpr)
  {
    auto opt_map = mapping_.slot(sexpr); // O(1)

    if (!opt_map.has_value())
      return nullptr;

    return memo_.data() + opt_map.value();
  }

  void store(LazyScalar& slot, scalar_type val)
  {
    assert(!slot.has_value() && "Already evaluated");
    assert(uintptr_t(&slot) >= uintptr_t(memo_.data()) && "Invalid slot pointer");
    assert(uintptr_t(&slot) < uintptr_t(memo_.data() + memo_.size()) && "Invalid slot pointer");

    slot = val;
  }

  void clear()
  {
     std::fill(memo_.begin(), memo_.end(), std::nullopt);
  }

private:
  std::vector<LazyScalar> memo_;
  const ReusedExprMapping &mapping_;
};

struct ExprEvaluator
{
  // stores where to update on stack runtime value
  // TODO: should we also handle stack updates lazily?
  using stack_t = EvalStack<scalar_type>;
  
  explicit ExprEvaluator(const Expr& e);

  scalar_type evaluate(EvalState& e, std::function<scalar_type (uintptr_t)> binding_fn) const;

  const ExprDAG& dag() const;
  const Expr& expr() const;

private:
  static scalar_type scalar_operand_value(Operand op) noexcept;

  void calculate_subops_operands(); // O(n) - n is number of operations
  size_t calculate_operands_range(size_t start, size_t end);

  void prepare_eval();
  void prepare_eval_operand(Operand op);

  void evaluate_subexpr(size_t op_index, stack_t& eval_stack, EvalState& es) const;


private:
  std::vector<Operation> operations_; // Inorder Depth First list of operations
  stack_t initial_stack_;
  std::vector<binding_gap_t> binding_gaps_;

  const Expr& expr_;
};

} // namespace glfdc
