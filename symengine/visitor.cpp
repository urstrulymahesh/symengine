#include <symengine/visitor.h>

#define ACCEPT(CLASS)                                                          \
    void CLASS::accept(Visitor &v) const                                       \
    {                                                                          \
        v.visit(*this);                                                        \
    }

namespace SymEngine
{

#define SYMENGINE_ENUM(TypeID, Class) ACCEPT(Class)
#include "symengine/type_codes.inc"
#undef SYMENGINE_ENUM

void preorder_traversal(const Basic &b, Visitor &v)
{
    b.accept(v);
    for (const auto &p : b.get_args())
        preorder_traversal(*p, v);
}

void postorder_traversal(const Basic &b, Visitor &v)
{
    for (const auto &p : b.get_args())
        postorder_traversal(*p, v);
    b.accept(v);
}

void preorder_traversal_stop(const Basic &b, StopVisitor &v)
{
    b.accept(v);
    if (v.stop_)
        return;
    for (const auto &p : b.get_args()) {
        preorder_traversal_stop(*p, v);
        if (v.stop_)
            return;
    }
}

void postorder_traversal_stop(const Basic &b, StopVisitor &v)
{
    for (const auto &p : b.get_args()) {
        postorder_traversal_stop(*p, v);
        if (v.stop_)
            return;
    }
    b.accept(v);
}

bool has_symbol(const Basic &b, const Symbol &x)
{
    // We are breaking a rule when using ptrFromRef() here, but since
    // HasSymbolVisitor is only instantiated and freed from here, the `x` can
    // never go out of scope, so this is safe.
    HasSymbolVisitor v(ptrFromRef(x));
    return v.apply(b);
}

RCP<const Basic> coeff(const Basic &b, const Basic &x, const Basic &n)
{
    if (!is_a<Symbol>(x)) {
        throw NotImplementedError("Not implemented for non Symbols.");
    }
    CoeffVisitor v(ptrFromRef(static_cast<const Symbol &>(x)), ptrFromRef(n));
    return v.apply(b);
}

class FreeSymbolsVisitor : public BaseVisitor<FreeSymbolsVisitor>
{
public:
    set_basic s;

    void bvisit(const Symbol &x)
    {
        s.insert(x.rcp_from_this());
    }

    void bvisit(const Subs &x)
    {
        set_basic set_ = free_symbols(*x.get_arg());
        for (const auto &p : x.get_variables()) {
            set_.erase(p);
        }
        s.insert(set_.begin(), set_.end());
        for (const auto &p : x.get_point()) {
            p->accept(*this);
        }
    }

    void bvisit(const Basic &x)
    {
        for (const auto &p : x.get_args()) {
            p->accept(*this);
        }
    }

    set_basic apply(const Basic &b)
    {
        b.accept(*this);
        return s;
    }
};

set_basic free_symbols(const Basic &b)
{
    FreeSymbolsVisitor visitor;
    return visitor.apply(b);
}

} // SymEngine
