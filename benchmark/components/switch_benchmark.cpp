#include <hsm/details/switch.h>

using namespace hsm;
using namespace boost::hana;

int main()
{
    switch_(case_(false_c, []() {}), case_(true_c, []() {}), case_(otherwise(), []() {}))();
}