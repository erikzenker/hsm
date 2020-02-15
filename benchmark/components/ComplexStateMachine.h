#include <hsm/hsm.h>

using namespace hsm;

struct e1 {
};
struct e2 {
};
struct e3 {
};
struct e4 {
};
struct e5 {
};
struct e6 {
};
struct e7 {
};
struct e8 {
};
struct e9 {
};
struct e10 {
};
struct e11 {
};
struct e12 {
};
struct e13 {
};
struct e14 {
};
struct e15 {
};
struct e16 {
};
struct e17 {
};
struct e18 {
};
struct e19 {
};
struct e20 {
};
struct e21 {
};
struct e22 {
};
struct e23 {
};
struct e24 {
};
struct e25 {
};
struct e26 {
};
struct e27 {
};
struct e28 {
};
struct e29 {
};
struct e30 {
};
struct e31 {
};
struct e32 {
};
struct e33 {
};
struct e34 {
};
struct e35 {
};
struct e36 {
};
struct e37 {
};
struct e38 {
};
struct e39 {
};
struct e40 {
};
struct e41 {
};
struct e42 {
};
struct e43 {
};
struct e44 {
};
struct e45 {
};
struct e46 {
};
struct e47 {
};
struct e48 {
};
struct e49 {
};
struct e50 {
};
struct e51 {
};


auto guard = [](auto /*event*/) { return true; };
auto action = [](auto /*event*/) {};

class idle {
};
class s1 {
};
class s2 {
};
class s3 {
};
class s4 {
};
class s5 {
};
class s6 {
};
class s7 {
};
class s8 {
};
class s9 {
};
class s10 {
};
class s11 {
};
class s12 {
};
class s13 {
};
class s14 {
};
class s15 {
};
class s16 {
};
class s17 {
};
class s18 {
};
class s19 {
};
class s20 {
};
class s21 {
};
class s22 {
};
class s23 {
};
class s24 {
};
class s25 {
};
class s26 {
};
class s27 {
};
class s28 {
};
class s29 {
};
class s30 {
};
class s31 {
};
class s32 {
};
class s33 {
};
class s34 {
};
class s35 {
};
class s36 {
};
class s37 {
};
class s38 {
};
class s39 {
};
class s40 {
};
class s41 {
};
class s42 {
};
class s43 {
};
class s44 {
};
class s45 {
};
class s46 {
};
class s47 {
};
class s48 {
};
class s49 {
};
class s50 {
};

    struct ComplexStateMachine {
        auto constexpr initial_state()
        {
            return initial(idle {});
        }

        auto constexpr make_transition_table()
        {
            return transition_table(
                row(idle {}, event<e1> {}, guard, action, s1 {}),
                row(s1 {}, event<e2> {}, guard, action, s2 {}),
                row(s2 {}, event<e3> {}, guard, action, s3 {}),
                row(s3 {}, event<e4> {}, guard, action, s4 {}),
                row(s4 {}, event<e5> {}, guard, action, s5 {}),
                row(s5 {}, event<e6> {}, guard, action, s6 {}),
                row(s6 {}, event<e7> {}, guard, action, s7 {}),
                row(s7 {}, event<e8> {}, guard, action, s8 {}),
                row(s8 {}, event<e9> {}, guard, action, s9 {}),
                row(s9 {}, event<e10> {}, guard, action, s10 {}),
                row(s10 {}, event<e11> {}, guard, action, s11 {}),
                row(s11 {}, event<e12> {}, guard, action, s12 {}),
                row(s12 {}, event<e13> {}, guard, action, s13 {}),
                row(s13 {}, event<e14> {}, guard, action, s14 {}),
                row(s14 {}, event<e15> {}, guard, action, s15 {}),
                row(s15 {}, event<e16> {}, guard, action, s16 {}),
                row(s16 {}, event<e17> {}, guard, action, s17 {}),
                row(s17 {}, event<e18> {}, guard, action, s18 {}),
                row(s18 {}, event<e19> {}, guard, action, s19 {}),
                row(s19 {}, event<e20> {}, guard, action, s20 {}),
                row(s20 {}, event<e21> {}, guard, action, s21 {}),
                row(s21 {}, event<e22> {}, guard, action, s22 {}),
                row(s22 {}, event<e23> {}, guard, action, s23 {}),
                row(s23 {}, event<e24> {}, guard, action, s24 {}),
                row(s24 {}, event<e25> {}, guard, action, s25 {}),
                row(s25 {}, event<e26> {}, guard, action, s26 {}),
                row(s26 {}, event<e27> {}, guard, action, s27 {}),
                row(s27 {}, event<e28> {}, guard, action, s28 {}),
                row(s28 {}, event<e29> {}, guard, action, s29 {}),
                row(s29 {}, event<e30> {}, guard, action, s30 {}),
                row(s30 {}, event<e31> {}, guard, action, s31 {}),
                row(s31 {}, event<e32> {}, guard, action, s32 {}),
                row(s32 {}, event<e33> {}, guard, action, s33 {}),
                row(s33 {}, event<e34> {}, guard, action, s34 {}),
                row(s34 {}, event<e35> {}, guard, action, s35 {}),
                row(s35 {}, event<e36> {}, guard, action, s36 {}),
                row(s36 {}, event<e37> {}, guard, action, s37 {}),
                row(s37 {}, event<e38> {}, guard, action, s38 {}),
                row(s38 {}, event<e39> {}, guard, action, s39 {}),
                row(s39 {}, event<e40> {}, guard, action, s40 {}),
                row(s40 {}, event<e41> {}, guard, action, s41 {}),
                row(s41 {}, event<e42> {}, guard, action, s42 {}),
                row(s42 {}, event<e43> {}, guard, action, s43 {}),
                row(s43 {}, event<e44> {}, guard, action, s44 {}),
                row(s44 {}, event<e45> {}, guard, action, s45 {}),
                row(s45 {}, event<e46> {}, guard, action, s46 {}),
                row(s46 {}, event<e47> {}, guard, action, s47 {}),
                row(s47 {}, event<e48> {}, guard, action, s48 {}),
                row(s48 {}, event<e49> {}, guard, action, s49 {}),
                row(s49 {}, event<e50> {}, guard, action, s50 {}),
                row(s50 {}, event<e51> {}, guard, action, idle {})
                );
        }
    };