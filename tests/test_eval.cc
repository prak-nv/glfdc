#include "../eval.hh"
#include "../expr_builder.hh"

#include "../cfold.hh"

#include "unknowns.hh"

#include "catch2/catch.hpp"

#include <iostream>

using namespace glfdc;

auto mk_op = [](char c) -> OperatorKind { return OperatorKind(c); };

TEST_CASE("Expression evalution", "[eval]")
{ 
   SECTION("ReusedExprMapping")
   {
     GIVEN("Eager mapping")
    {
      auto eager_map = ReusedExprMapping::create_eager_mapping();
      THEN("No reused expression")
      {
        REQUIRE(eager_map.empty());
      }
    }
  }

  SECTION("ExprEvaluator")
  {
     auto eager_map = ReusedExprMapping::create_eager_mapping();

     GIVEN("an empty eval state")
     {
       EvalState es(eager_map);

       auto op1 = GENERATE(mk_op('+'), mk_op('-'), mk_op('*'), mk_op('/'), mk_op('%'));
       auto op2 = GENERATE(mk_op('+'), mk_op('-'), mk_op('*'), mk_op('/'), mk_op('%'));
       auto op3 = GENERATE(mk_op('+'), mk_op('-'), mk_op('*'), mk_op('/'), mk_op('%'));

       auto i = GENERATE(1, 2, 3, -10, -3, -2, -1);
       auto j = GENERATE(1, 2, 3, -10, -3, -2, -1);

       DYNAMIC_SECTION("Expression (x " << char(op1) << " " << i <<") "
                       << char(op3) << " (y " << char(op2) << " " << j << ")")
       {
         ExpressionBuilder builder;

         auto test_unkwns = alpahabetic_unknowns();

#        define DECLARE_unknown(_n) auto _n = test_unkwns.get_by_name(#_n); auto u ## _n = builder.get_binding(_n);
         DECLARE_unknown(x);
         DECLARE_unknown(y);
#        undef DECLARE_unknown

         auto s_1 = builder.create_sexpr(op1, ux, Value(i));
         auto s_2 = builder.create_sexpr(op2, uy, Value(j));
         auto s_1_2 = builder.create_sexpr(op3, s_1, s_2);

         auto e_1 = builder.create_expr(s_1);
         auto e_2 = builder.create_expr(s_2);
         auto e_1_2 = builder.create_expr(s_1_2);

         ExprEvaluator eval_e_1(e_1.value());
         ExprEvaluator eval_e_2(e_2.value());
         ExprEvaluator eval_e_1_2(e_1_2.value());

         int result_e1 = eval_e_1.evaluate(es, unknown_value);

         THEN("result of e_1")
         {
           REQUIRE(result_e1 == cfold(op1, 0, i));
         }

         int result_e2 = eval_e_2.evaluate(es, unknown_value);

         THEN("result of e_2")
         {
           REQUIRE(result_e2 == cfold(op2, 0, j));
         }
         
         bool is_null_div = result_e2 == 0 && (op3 == OperatorKind::div || op3 == OperatorKind::mod);

         int result_e12 = is_null_div? 0 : eval_e_1_2.evaluate(es, unknown_value);

         THEN("result of e_2")
         {
           REQUIRE(result_e12 == cfold(op3, result_e1, result_e2));
         }

         GIVEN("lazy mapping")
         {
            ExpressionBuilder builder;

#        define DECLARE_unknown(_n) auto _n = test_unkwns.get_by_name(#_n); auto u ## _n = builder.get_binding(_n);
         DECLARE_unknown(x);
         DECLARE_unknown(y);
#        undef DECLARE_unknown
            auto s_1 = builder.create_sexpr(op1, ux, Value(i));
            auto s_1_1 = builder.create_sexpr(op1, s_1, s_1);
            auto e_1_1 = builder.create_expr(s_1_1);

            auto lazy_mapping = std::apply(ReusedExprMapping::create_lazy_mapping, builder.reuses());

            REQUIRE(lazy_mapping.size() == 1);
            EvalState lazy_es(lazy_mapping);
            ExprEvaluator eval_e_1_1(e_1_1.value());
         }
       }
     }
  }
}
