#include "hsm/hsm.h"

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

auto guard = [](auto /*event*/, auto /*source*/, auto /*target*/) { return true; };
auto action = [](auto /*event*/, auto /*source*/, auto /*target*/) {};

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
        static constexpr auto make_transition_table()
        {
            return transition_table(
                *state<idle> {} + event<e1> {}[guard] / action = state<s1> {},
                state<s1> {} + event<e2> {}[guard] / action = state<s2> {},
                state<s2> {} + event<e3> {}[guard] / action = state<s3> {},
                state<s3> {} + event<e4> {}[guard] / action = state<s4> {},
                state<s4> {} + event<e5> {}[guard] / action = state<s5> {},
                state<s5> {} + event<e6> {}[guard] / action = state<s6> {},
                state<s6> {} + event<e7> {}[guard] / action = state<s7> {},
                state<s7> {} + event<e8> {}[guard] / action = state<s8> {},
                state<s8> {} + event<e9> {}[guard] / action = state<s9> {},
                state<s9> {} + event<e10> {}[guard] / action = state<s10> {},
                state<s10> {} + event<e11> {}[guard] / action = state<s11> {},
                state<s11> {} + event<e12> {}[guard] / action = state<s12> {},
                state<s12> {} + event<e13> {}[guard] / action = state<s13> {},
                state<s13> {} + event<e14> {}[guard] / action = state<s14> {},
                state<s14> {} + event<e15> {}[guard] / action = state<s15> {},
                state<s15> {} + event<e16> {}[guard] / action = state<s16> {},
                state<s16> {} + event<e17> {}[guard] / action = state<s17> {},
                state<s17> {} + event<e18> {}[guard] / action = state<s18> {},
                state<s18> {} + event<e19> {}[guard] / action = state<s19> {},
                state<s19> {} + event<e20> {}[guard] / action = state<s20> {},
                state<s20> {} + event<e21> {}[guard] / action = state<s21> {},
                state<s21> {} + event<e22> {}[guard] / action = state<s22> {},
                state<s22> {} + event<e23> {}[guard] / action = state<s23> {},
                state<s23> {} + event<e24> {}[guard] / action = state<s24> {},
                state<s24> {} + event<e25> {}[guard] / action = state<s25> {},
                state<s25> {} + event<e26> {}[guard] / action = state<s26> {},
                state<s26> {} + event<e27> {}[guard] / action = state<s27> {},
                state<s27> {} + event<e28> {}[guard] / action = state<s28> {},
                state<s28> {} + event<e29> {}[guard] / action = state<s29> {},
                state<s29> {} + event<e30> {}[guard] / action = state<s30> {},
                state<s30> {} + event<e31> {}[guard] / action = state<s31> {},
                state<s31> {} + event<e32> {}[guard] / action = state<s32> {},
                state<s32> {} + event<e33> {}[guard] / action = state<s33> {},
                state<s33> {} + event<e34> {}[guard] / action = state<s34> {},
                state<s34> {} + event<e35> {}[guard] / action = state<s35> {},
                state<s35> {} + event<e36> {}[guard] / action = state<s36> {},
                state<s36> {} + event<e37> {}[guard] / action = state<s37> {},
                state<s37> {} + event<e38> {}[guard] / action = state<s38> {},
                state<s38> {} + event<e39> {}[guard] / action = state<s39> {},
                state<s39> {} + event<e40> {}[guard] / action = state<s40> {},
                state<s40> {} + event<e41> {}[guard] / action = state<s41> {},
                state<s41> {} + event<e42> {}[guard] / action = state<s42> {},
                state<s42> {} + event<e43> {}[guard] / action = state<s43> {},
                state<s43> {} + event<e44> {}[guard] / action = state<s44> {},
                state<s44> {} + event<e45> {}[guard] / action = state<s45> {},
                state<s45> {} + event<e46> {}[guard] / action = state<s46> {},
                state<s46> {} + event<e47> {}[guard] / action = state<s47> {},
                state<s47> {} + event<e48> {}[guard] / action = state<s48> {},
                state<s48> {} + event<e49> {}[guard] / action = state<s49> {},
                state<s49> {} + event<e50> {}[guard] / action = state<s50> {},
                state<s50> {} + event<e51> {}[guard] / action = state<idle> {});
        }
    };