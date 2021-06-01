#include "catch2/catch.hpp"

#include "stack.hh"

using namespace glfdc;

TEST_CASE("EvalStack", "[eval]")
{
  EvalStack<int> stack;

  GIVEN("an empty stack")
  {

    THEN("stack is empty")
    {
      REQUIRE(stack.size() == 0);
      REQUIRE(stack.empty());
    }

    WHEN("push(1) is called")
    {
      stack.push(1);

      THEN("contains element")
      {
        REQUIRE(stack.top() == 1);
        REQUIRE(stack.size() == 1);
        REQUIRE(!stack.empty());
      }
    }

    WHEN("push(2) is called")
    {
      stack.push(2);

      THEN("contains 1 elements")
      {
        REQUIRE(stack.top() == 2);
        REQUIRE(!stack.empty());
        REQUIRE(stack.size() == 1);
      }

      AND_WHEN("push(3) is called")
      {
        stack.push(3);
        THEN("contains 3 elements")
        {
          REQUIRE(stack.size() == 2);
          REQUIRE(!stack.empty());
          REQUIRE(stack.top() == 3);
        }
      
        AND_WHEN("pop_top() is called")
        {
          int value = stack.pop_top();
      
          THEN("contains 1 elements")
          {
            REQUIRE(stack.size() == 1);
            REQUIRE(!stack.empty());
            REQUIRE(stack.top() == 2);
            REQUIRE(value == 3);
          }

          AND_WHEN("push(3) is called")
          {
            stack.push(3);
            THEN("contains 2 elements")
            {
              REQUIRE(stack.size() == 2);
              REQUIRE(!stack.empty());
              REQUIRE(stack.top() == 3);
            }

            AND_WHEN("drop(1) is called")
            {
              stack.drop(1);

              THEN("contains element")
              {
                REQUIRE(stack.size() == 1);
                REQUIRE(!stack.empty());
                REQUIRE(stack.top() == 2);
              }
            
              AND_WHEN("pop() is called")
              {
                stack.pop();
              
                THEN("contains no elements")
                {
                  REQUIRE(stack.size() == 0);
                  REQUIRE(stack.empty());
                }
              }
            }
          }
        }
      }
    }
  }
}
