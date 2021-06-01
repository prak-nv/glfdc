#include "catch2/catch.hpp"

#include "expr_builder.hh"

#include "unknowns.hh"

#include <iostream>
#include <vector>

using namespace glfdc;

auto mk_op = [](char c) -> OperatorKind { return OperatorKind(c); };

std::string operator_str(OperatorKind oper)
{
  std::string ret{1, '\0'};
  ret[0] = char(oper);

  return ret;
}

TEST_CASE("Verify primitive subexpr", "[build]")
{
  SECTION("Constant scalar expr eager evaluation")
  {
    ExpressionBuilder builder;

    auto op = GENERATE(mk_op('+'), mk_op('-'), mk_op('*'), mk_op('/'), mk_op('%'));

    auto i = GENERATE(0, 1, 2, 3, -10, -3, -2, -1);
    auto j = GENERATE(1, 2, 3, -10, -3, -2, -1);

    DYNAMIC_SECTION("Expression: " << i << operator_str(op) << j)
    {
      auto operand = builder.create_sexpr(op, Value(i), Value(j));
      THEN("result operand is still scalar")
      {
        REQUIRE(is_value(operand));
        REQUIRE(is_scalar(operand));
        REQUIRE(!is_unbound_value(operand));
        REQUIRE(!is_sexpr(operand));
      }
    }

    DYNAMIC_SECTION("Value of" << i << "+" << j)
    {
      auto operand = builder.create_sexpr(mk_op('+'), Value(i), Value(j));
      auto operand_comm = builder.create_sexpr(mk_op('+'), Value(j), Value(i));

      THEN("result is correctly calculated")
      {
        REQUIRE(operand == Operand(Value(i + j)));
        REQUIRE(operand_comm == Operand(Value(j + i)));
      }
      THEN("result is commutative")
      {
        REQUIRE(operand_comm == operand);
      }
    }

    DYNAMIC_SECTION("Value of" << i << "-" << j)
    {
      auto operand = builder.create_sexpr(mk_op('-'), Value(i), Value(j));

      THEN("result is correctly calculated")
      {
        REQUIRE(operand == Operand(Value(i - j)));
      }
    }

    DYNAMIC_SECTION("Value of" << i << "*" << j)
    {
      auto operand = builder.create_sexpr(mk_op('*'), Value(i), Value(j));
      auto operand_comm = builder.create_sexpr(mk_op('*'), Value(j), Value(i));

      THEN("result is correctly calculated")
      {
        REQUIRE(operand == Operand(Value(i * j)));
        REQUIRE(operand_comm == Operand(Value(j * i)));
      }
      THEN("result is commutative")
      {
        REQUIRE(operand_comm == operand);
      }
    }

    DYNAMIC_SECTION("Value of" << i << "/" << j)
    {
      auto operand = builder.create_sexpr(mk_op('/'), Value(i), Value(j));

      THEN("result is correctly calculated")
      {
        REQUIRE(operand == Operand(Value(i / j)));
      }
    }

    DYNAMIC_SECTION("Value of" << i << "%" << j)
    {
      auto operand = builder.create_sexpr(mk_op('%'), Value(i), Value(j));

      THEN("result is correctly calculated")
      {
        REQUIRE(operand == Operand(Value(i % j)));
      }
    }
  }

  SECTION("Expressions with unknowns")
  {
    // Operators:
    auto op = GENERATE(mk_op('+'), mk_op('-'), mk_op('*'), mk_op('/'), mk_op('%'));

    // Scalars:
    auto i = GENERATE(0, 1, 2, 3, -10, -3, -2, -1);
    auto j = GENERATE(1, 2, 3, -10, -3, -2, -1);

    // Unbound (runtime) values
    const Unknowns test_unk{6};

    GIVEN("Unknown values a, b and c")
    {
      auto unk = GENERATE_REF(take(3, unknowns(test_unk)));
     
      DYNAMIC_SECTION("Expr " << i << operator_str(op) << test_unk.name(unk))
      {
        ExpressionBuilder builder;
     
        Value unbnd = builder.get_binding(unk);
        auto operand = builder.create_sexpr(op, Value(i), unbnd);
     
        THEN("result operand is subexpression")
        {
          REQUIRE(is_sexpr(operand));
          REQUIRE(!is_value(operand));
          REQUIRE(is_lref(std::get<SExprRef>(operand)));
        }
      }
     
      DYNAMIC_SECTION("Expr " << test_unk.name(unk) << operator_str(op) << j)
      {
        ExpressionBuilder builder;
     
        Value unbnd = builder.get_binding(unk);
        auto operand = builder.create_sexpr(op, unbnd, Value(j));
     
        THEN("result operand is subexpression")
        {
          REQUIRE(is_sexpr(operand));
          REQUIRE(!is_value(operand));
          REQUIRE(is_lref(std::get<SExprRef>(operand)));
        }
      }
     
      DYNAMIC_SECTION("Expr " << test_unk.name(unk) << operator_str(op) << j)
      {
        ExpressionBuilder builder;
     
        Value unbnd = builder.get_binding(unk);
        auto operand = builder.create_sexpr(op, unbnd, Value(j));
     
        THEN("result operand is subexpression")
        {
          REQUIRE(is_sexpr(operand));
          REQUIRE(!is_value(operand));
          REQUIRE(is_lref(std::get<SExprRef>(operand)));
        }
      }
     
      DYNAMIC_SECTION("Expr " << test_unk.name(unk) << operator_str(op) << test_unk.name(unk))
      {
        ExpressionBuilder builder;
     
        Value unbnd = builder.get_binding(unk);
        auto operand = builder.create_sexpr(op, unbnd, unbnd);
     
        THEN("result operand is subexpression")
        {
          REQUIRE(is_sexpr(operand));
          REQUIRE(!is_value(operand));
          REQUIRE(is_lref(std::get<SExprRef>(operand)));
        }
      }
    }


    GIVEN("Unknowns d, e, f")
    {
      auto d = test_unk.get_by_name("d");
      auto e = test_unk.get_by_name("e");
      auto f = test_unk.get_by_name("f");

      ExpressionBuilder builder;

      auto ud = builder.get_binding(d);
      auto ue = builder.get_binding(e);
      auto uf = builder.get_binding(f);

      DYNAMIC_SECTION("for expressions: " << i << operator_str(op) << "d"
                      " and " << i << operator_str(op) << "e")
      {
        auto sexpr_id = builder.create_sexpr(op, Value(i), ud);
        auto sexpr_ie = builder.create_sexpr(op, Value(i), ue);

        THEN("expressions are distinct")
        {
          REQUIRE(sexpr_id != sexpr_ie);
        }
      }

      DYNAMIC_SECTION("for expressions: d" << operator_str(op) << j <<
                      " and e" << operator_str(op) << j)
      {
        auto sexpr_dj = builder.create_sexpr(op, ud, Value(j));
        auto sexpr_ej = builder.create_sexpr(op, ue, Value(j));

        THEN("expressions are distinct")
        {
          REQUIRE(sexpr_dj != sexpr_ej);
        }
      }

      DYNAMIC_SECTION("for expressions: d" << operator_str(op) << "f" <<
                      " and e" << operator_str(op) << "f")
      {
        auto sexpr_df = builder.create_sexpr(op, ud, uf);
        auto sexpr_ef = builder.create_sexpr(op, ue, uf);

        THEN("expressions are distinct")
        {
          REQUIRE(sexpr_df != sexpr_ef);
        }
      }

      DYNAMIC_SECTION("for expressions: f" << operator_str(op) << "d" <<
                      " and f" << operator_str(op) << "e")
      {
        auto sexpr_fd = builder.create_sexpr(op, uf, ud);
        auto sexpr_fe = builder.create_sexpr(op, uf, ue);

        THEN("expressions are distinct")
        {
          REQUIRE(sexpr_fd != sexpr_fe);
        }
      }
    }

    GIVEN("Unknown d, e, f and equivalence e <=> f")
    {
      auto d = test_unk.get_by_name("d");
      auto e = test_unk.get_by_name("e");
      auto f = test_unk.get_by_name("f");

      ExpressionBuilder builder;
 
      auto ud = builder.get_binding(d);
      auto ue = builder.get_binding(e);
      auto uf = builder.add_binding_equivalence(e, f);
 
      DYNAMIC_SECTION("for expressions: " << i << operator_str(op) << "e"
                      " and " << i << operator_str(op) << "f")
      {
        auto sexpr_ie = builder.create_sexpr(op, Value(i), ue);
        auto sexpr_if = builder.create_sexpr(op, Value(i), ue);
 
        THEN("expressions are the same")
        {
          REQUIRE(sexpr_ie == sexpr_if);
        }
      }

      DYNAMIC_SECTION("for expressions: e" << operator_str(op) << j <<
                      " and f" << operator_str(op) << j)
      {
        auto sexpr_ej = builder.create_sexpr(op, ue, Value(j));
        auto sexpr_fj = builder.create_sexpr(op, uf, Value(j));
     
        THEN("expressions are the same")
        {
          REQUIRE(sexpr_ej == sexpr_fj);
        }
      }

      DYNAMIC_SECTION("for expressions: d" << operator_str(op) << "e" <<
                      " and d" << operator_str(op) << "f")
      {
        auto sexpr_de = builder.create_sexpr(op, ud, ue);
        auto sexpr_df = builder.create_sexpr(op, ud, uf);

        THEN("expressions are the same")
        {
          REQUIRE(sexpr_de == sexpr_df);
        }
      }

      DYNAMIC_SECTION("for expressions: f" << operator_str(op) << "d" <<
                      " and f" << operator_str(op) << "e")
      {
        auto sexpr_fd = builder.create_sexpr(op, uf, ud);
        auto sexpr_fe = builder.create_sexpr(op, uf, ue);

        THEN("expressions are the same")
        {
          REQUIRE(sexpr_fd == sexpr_fd);
        }
      }
    }

    GIVEN("Unknowns d, e")
    {
      ExpressionBuilder builder;

      auto d = test_unk.get_by_name("d");
      auto e = test_unk.get_by_name("e");

      auto ud = builder.get_binding(d);
      auto ue = builder.get_binding(e);

      AND_GIVEN("commutative operator")
      {
        auto op_comm = GENERATE(mk_op('+'), mk_op('*'));

        DYNAMIC_SECTION("for expression: " << j << operator_str(op_comm) << "d"
                        " and d" << operator_str(op_comm) << j)
        {
          auto sexpr_dj = builder.create_sexpr(op_comm, ud, Value(j));
          auto sexpr_jd = builder.create_sexpr(op_comm, Value(j), ud);
        
          THEN("expressions are equivalent")
          {
            REQUIRE(sexpr_dj == sexpr_jd);
          }
        }
        
        DYNAMIC_SECTION("for expression: d" << operator_str(op_comm) << "e "
                        "and e" << operator_str(op_comm) << "d")
        {
          auto sexpr_ed = builder.create_sexpr(op_comm, ue, ud);
          auto sexpr_de = builder.create_sexpr(op_comm, ud, ue);
        
          THEN("expressions are equivalent")
          {
            REQUIRE(sexpr_de == sexpr_ed);
          }
        }
      }

      AND_GIVEN("noncommutative operator")
      {
        auto op_noncomm = GENERATE(mk_op('-'), mk_op('/'), mk_op('%'));

        DYNAMIC_SECTION("for expression: " << j << operator_str(op_noncomm) << "d"
                        " and d" << operator_str(op_noncomm) << j)
        {
          auto sexpr_dj = builder.create_sexpr(op_noncomm, ud, Value(j));
          auto sexpr_jd = builder.create_sexpr(op_noncomm, Value(j), ud);
        
          THEN("expressions are not equivalent")
          {
            REQUIRE(sexpr_dj != sexpr_jd);
          }
        }
        
        DYNAMIC_SECTION("for expression: d" << operator_str(op_noncomm) << "e "
                       "and e" << operator_str(op_noncomm) << "d")
        {
          auto sexpr_ed = builder.create_sexpr(op_noncomm, ue, ud);
          auto sexpr_de = builder.create_sexpr(op_noncomm, ud, ue);
        
          THEN("expressions are not equivalent")
          {
            REQUIRE(sexpr_de != sexpr_ed);
          }
        }

      }
    }
  }
}

TEST_CASE("Complex exprs", "[build]")
{
  SECTION("mutilevel exprs")
  {
    ExpressionBuilder builder;
    auto test_unkwns = alpahabetic_unknowns(); // All single latin letter unknowns

    GIVEN("Unknowns x, y, z")
    {
#     define DECLARE_unknown(_n) auto _n = test_unkwns.get_by_name(#_n); auto u ## _n = builder.get_binding(_n);
      DECLARE_unknown(x);
      DECLARE_unknown(y);
      DECLARE_unknown(z);
#     undef DECLARE_unknown

      AND_GIVEN("subexpressions x+1 y*2 z/3")
      {
        auto x_plus_1 = builder.create_sexpr(mk_op('+'), ux, Value(1));
        auto x_minus_y = builder.create_sexpr(mk_op('-'), ux, uy);
        auto y_times_2 = builder.create_sexpr(mk_op('*'), uy, Value(2));
        auto z_divby_3 = builder.create_sexpr(mk_op('/'), uz, Value(3));

        auto sub_expr = GENERATE_COPY(x_plus_1, x_minus_y, y_times_2, z_divby_3);

        // XXX: missing 'z % x' so we have one distinct expression
        auto z_mod_x = builder.create_sexpr(mk_op('%'), uz, ux);

        AND_GIVEN("any operator")
        {
          auto op = GENERATE(mk_op('+'), mk_op('-'), mk_op('*'), mk_op('/'), mk_op('%'));

          WHEN("applying scalar")
          {
            auto i = GENERATE(1, 2, 3, -10, -3, -2, -1);

            THEN("result is internal reference")
            {
              auto ei = builder.create_sexpr(op, sub_expr, Value(i));

              REQUIRE(is_sexpr(ei));
              REQUIRE(is_iref(std::get<SExprRef>(ei)));

              auto ie = builder.create_sexpr(op, Value(i), sub_expr);

              REQUIRE(is_sexpr(ie));
              REQUIRE(is_iref(std::get<SExprRef>(ie)));
            }
          }

          WHEN("applying un(bound/known)")
          {
            auto u = GENERATE_COPY(ux, uy, uz);

            THEN("result is leaf reference")
            {
              auto eu = builder.create_sexpr(op, sub_expr, u);

              REQUIRE(is_sexpr(eu));
              REQUIRE(is_lref(std::get<SExprRef>(eu)));

              auto ue = builder.create_sexpr(op, u, sub_expr);

              REQUIRE(is_sexpr(ue));
              REQUIRE(is_lref(std::get<SExprRef>(ue)));
            }
          }

          WHEN("applying subexpr")
          {
            THEN("result is internal reference")
            {
              auto ee1 = builder.create_sexpr(op, sub_expr, x_plus_1);

              REQUIRE(is_sexpr(ee1));
              REQUIRE(is_iref(std::get<SExprRef>(ee1)));

              auto ee2 = builder.create_sexpr(op, z_mod_x, sub_expr);

              REQUIRE(is_sexpr(ee2));
              REQUIRE(is_iref(std::get<SExprRef>(ee2)));
            }
          }

          WHEN("applying same subexpr")
          {
            THEN("left and right are the same")
            {
              auto ee = builder.create_sexpr(op, sub_expr, sub_expr);
              auto se = builder.dag().fetch(std::get<SExprRef>(ee));
              REQUIRE(se.lhs_ == se.rhs_);
            }
          }

          WHEN("applying distinct subexpr")
          {
            THEN("left and right are distinct")
            {
              {
                auto ee1 = builder.create_sexpr(op, sub_expr, z_mod_x);
                auto se1 = builder.dag().fetch(std::get<SExprRef>(ee1));
                REQUIRE(se1.lhs_ != se1.rhs_);
              }

              {
                auto ee2 = builder.create_sexpr(op, z_mod_x, sub_expr);
                auto se2 = builder.dag().fetch(std::get<SExprRef>(ee2));
                REQUIRE(se2.lhs_ != se2.rhs_);
              }
            }
          }
        }

        AND_GIVEN("commutative operator")
        {
          auto op_comm = GENERATE(mk_op('+'), mk_op('*'));
        
          WHEN("applying scalar")
          {
            auto i = GENERATE(1, 2, 3, -10, -3, -2, -1);
        
            THEN("result expressions are the same")
            {
              auto ei = builder.create_sexpr(op_comm, sub_expr, Value(i));
              auto ie = builder.create_sexpr(op_comm, Value(i), sub_expr);
        
              REQUIRE(ei == ie);
            }
          }
        
          WHEN("applying un(bound/known)")
          {
            auto u = GENERATE_COPY(ux, uy, uz);
        
            THEN("result expressions are the same")
            {
              auto eu = builder.create_sexpr(op_comm, sub_expr, u);
              auto ue = builder.create_sexpr(op_comm, u, sub_expr);

              REQUIRE(eu == ue);
            }
          }
        
          WHEN("applying subexpr")
          {
            THEN("result expressions are the same")
            {
              auto ee1 = builder.create_sexpr(op_comm, sub_expr, z_mod_x);
              auto ee2 = builder.create_sexpr(op_comm, z_mod_x, sub_expr);
        
              REQUIRE(ee1 == ee2);
            }
          }
        }

        AND_GIVEN("noncommutative operator")
        {
          auto op_noncomm = GENERATE(mk_op('-'), mk_op('/'), mk_op('%'));
        
          WHEN("applying scalar")
          {
            auto i = GENERATE(1, 2, 3, -10, -3, -2, -1);
        
            THEN("result expressions are the same")
            {
              auto ei = builder.create_sexpr(op_noncomm, sub_expr, Value(i));
              auto ie = builder.create_sexpr(op_noncomm, Value(i), sub_expr);
        
              REQUIRE(ei != ie);
            }
          }
        
          WHEN("applying un(bound/known)")
          {
            auto u = GENERATE_COPY(ux, uy, uz);
        
            THEN("result expressions are the same")
            {
              auto eu = builder.create_sexpr(op_noncomm, sub_expr, u);
              auto ue = builder.create_sexpr(op_noncomm, u, sub_expr);

              REQUIRE(eu != ue);
            }
          }
        
          WHEN("applying subexpr")
          {
            THEN("result expressions are the same")
            {
              auto ee1 = builder.create_sexpr(op_noncomm, sub_expr, z_mod_x);
              auto ee2 = builder.create_sexpr(op_noncomm, z_mod_x, sub_expr);
        
              REQUIRE(ee1 != ee2);
            }
          }
        }

      }
    }
  }
}
