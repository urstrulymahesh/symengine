#include "catch.hpp"

#include <symengine/logic.h>
#include <symengine/add.h>
#include <symengine/real_double.h>
#include <symengine/symengine_exception.h>

using SymEngine::SymEngineException;
using SymEngine::Basic;
using SymEngine::Integer;
using SymEngine::integer;
using SymEngine::real_double;
using SymEngine::Interval;
using SymEngine::interval;
using SymEngine::symbol;
using SymEngine::piecewise;
using SymEngine::contains;
using SymEngine::Contains;
using SymEngine::add;
using SymEngine::boolTrue;
using SymEngine::boolFalse;
using SymEngine::is_a;
using SymEngine::vec_basic;
using SymEngine::unified_eq;
using SymEngine::zero;
using SymEngine::one;
using SymEngine::eq;
using SymEngine::boolean;
using SymEngine::logical_and;
using SymEngine::logical_or;
using SymEngine::logical_not;
using SymEngine::set_boolean;

TEST_CASE("BooleanAtom : Basic", "[basic]")
{
    REQUIRE(boolTrue->__str__() == "True");
    REQUIRE(boolFalse->__str__() == "False");

    vec_basic v = boolTrue->get_args();
    vec_basic u;
    REQUIRE(unified_eq(v, u));

    auto x = symbol("x");
    CHECK_THROWS_AS(boolTrue->diff(x), SymEngineException);

    REQUIRE(not eq(*boolTrue, *boolFalse));
    REQUIRE(eq(*boolFalse, *boolean(false)));
}

TEST_CASE("Contains", "[logic]")
{
    auto x = symbol("x");
    auto y = symbol("y");
    auto int1 = interval(integer(1), integer(2), false, false);

    auto p = contains(real_double(1.5), int1);
    REQUIRE(eq(*p, *boolTrue));

    p = contains(integer(3), int1);
    REQUIRE(eq(*p, *boolFalse));

    p = contains(x, int1);
    REQUIRE(is_a<Contains>(*p));
    REQUIRE(p->__str__() == "Contains(x, [1, 2])");
    REQUIRE(eq(*p, *p));

    vec_basic v = p->get_args();
    vec_basic u = {x, int1};
    REQUIRE(unified_eq(v, u));

    CHECK_THROWS_AS(p->diff(x), SymEngineException);
}

TEST_CASE("Piecewise", "[logic]")
{
    auto x = symbol("x");
    auto y = symbol("y");
    auto int1 = interval(integer(1), integer(2), true, false);
    auto int2 = interval(integer(2), integer(5), true, false);
    auto int3 = interval(integer(5), integer(10), true, false);
    auto p = piecewise({{x, contains(x, int1)},
                        {y, contains(x, int2)},
                        {add(x, y), contains(x, int3)}});

    std::string s = "Piecewise((x, Contains(x, (1, 2])), (y, Contains(x, (2, "
                    "5])), (x + y, Contains(x, (5, 10])))";
    REQUIRE(s == p->__str__());

    auto q = piecewise({{one, contains(x, int1)},
                        {zero, contains(x, int2)},
                        {one, contains(x, int3)}});

    REQUIRE(eq(*p->diff(x), *q));
}

TEST_CASE("And, Or : Basic", "[basic]")
{
    set_boolean e;
    REQUIRE(eq(*logical_and(e), *boolTrue));
    REQUIRE(eq(*logical_or(e), *boolFalse));

    REQUIRE(eq(*logical_and({boolTrue}), *boolTrue));
    REQUIRE(eq(*logical_or({boolTrue}), *boolTrue));
    REQUIRE(eq(*logical_and({boolFalse}), *boolFalse));
    REQUIRE(eq(*logical_or({boolFalse}), *boolFalse));

    REQUIRE(eq(*logical_and({boolTrue, boolTrue}), *boolTrue));
    REQUIRE(eq(*logical_and({boolTrue, boolFalse}), *boolFalse));
    REQUIRE(eq(*logical_and({boolFalse, boolTrue}), *boolFalse));
    REQUIRE(eq(*logical_and({boolFalse, boolFalse}), *boolFalse));

    REQUIRE(eq(*logical_or({boolTrue, boolTrue}), *boolTrue));
    REQUIRE(eq(*logical_or({boolTrue, boolFalse}), *boolTrue));
    REQUIRE(eq(*logical_or({boolFalse, boolTrue}), *boolTrue));
    REQUIRE(eq(*logical_or({boolFalse, boolFalse}), *boolFalse));

    auto x = symbol("x");
    auto int1 = interval(integer(1), integer(2), false, false);
    auto int2 = interval(integer(1), integer(5), false, false);
    auto c1 = contains(x, int1);
    auto c2 = contains(x, int2);

    auto s1 = logical_and({c1, c2});
    std::string str = s1->__str__();
    REQUIRE(str.find("And(") == 0);
    REQUIRE(str.find(c1->__str__()) != std::string::npos);
    REQUIRE(str.find(c2->__str__()) != std::string::npos);
    auto s2 = logical_and({c2, c1});
    REQUIRE(s1->__hash__() == s2->__hash__());
    REQUIRE(eq(*s1, *s2));
    s1 = logical_or({c1, c2});
    str = s1->__str__();
    REQUIRE(str.find("Or(") == 0);
    REQUIRE(str.find(c1->__str__()) != std::string::npos);
    REQUIRE(str.find(c2->__str__()) != std::string::npos);
    s2 = logical_or({c2, c1});
    REQUIRE(s1->__hash__() == s2->__hash__());
    REQUIRE(eq(*s1, *s2));

    REQUIRE(eq(*logical_and({c1}), *c1));
    REQUIRE(eq(*logical_or({c1}), *c1));

    REQUIRE(eq(*logical_and({c1, logical_not(c1)}), *boolFalse));
    REQUIRE(eq(*logical_or({c1, logical_not(c1)}), *boolTrue));

    REQUIRE(eq(*logical_and({c1, boolTrue}), *c1));
    REQUIRE(eq(*logical_and({c1, boolFalse}), *boolFalse));
    REQUIRE(eq(*logical_or({c1, boolTrue}), *boolTrue));
    REQUIRE(eq(*logical_or({c1, boolFalse}), *c1));

    REQUIRE(eq(*logical_and({c1, c1, c2}), *logical_and({c1, c2})));
    REQUIRE(eq(*logical_or({c1, c1, c2}), *logical_or({c1, c2})));

    auto y = symbol("y");
    auto c3 = contains(y, int1);
    auto c4 = contains(y, int2);
    REQUIRE(eq(*logical_and({c1, c1, c2}), *logical_and({c1, c2})));
    REQUIRE(eq(*logical_and({logical_and({c1, c2}), logical_and({c3, c4})}),
               *logical_and({c1, c2, c3, c4})));
    REQUIRE(eq(
        *logical_and(
            {logical_and({c1, logical_and({c2, logical_and({c3, c4})})}), c2}),
        *logical_and({c1, c2, c3, c4})));
    REQUIRE(eq(*logical_or({c2, c1, c2}), *logical_or({c1, c2})));
    REQUIRE(eq(*logical_or({logical_or({c1, c2}), logical_or({c3, c4})}),
               *logical_or({c1, c2, c3, c4})));
    REQUIRE(eq(
        *logical_or({c1, logical_and({c2, c3, c4}), logical_and({c2, c4}),
                     logical_and({c2, c3, c4}), c1, logical_and({c2, c4})}),
        *logical_or({c1, logical_and({c2, c3, c4}), logical_and({c2, c4})})));
}

TEST_CASE("Not : Basic", "[basic]")
{
    auto x = symbol("x");
    auto int1 = interval(integer(1), integer(2), false, false);
    auto int2 = interval(integer(1), integer(5), false, false);
    auto c1 = contains(x, int1);
    auto c2 = contains(x, int2);

    REQUIRE(eq(*logical_not(boolTrue), *boolFalse));
    REQUIRE(eq(*logical_not(boolFalse), *boolTrue));
    REQUIRE(logical_not(c1)->__str__() == "Not(Contains(x, [1, 2]))");
    REQUIRE(eq(*logical_not(logical_and({c1, c2})),
               *logical_or({logical_not(c1), logical_not(c2)})));
    REQUIRE(eq(*logical_not(logical_or({c1, c2})),
               *logical_and({logical_not(c1), logical_not(c2)})));
}