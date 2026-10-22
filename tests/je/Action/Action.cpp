#include <assert.h>
#include "JE/Action.hpp"

// Standalone-shape check for the Action base flag-init recovered by R-FINISH item 6:
// no standalone Action::ctor exists in the image (f3_method_index over .?AUAction@@);
// every derived ctor inlines the base init as a LAST write mbr_0x4 = 1
// (ActionAnim @0x5a9252, ActionDie @0x55fb21, ActionIdle @0x560191, ActionMessage @0x5a9018).
// The port base must therefore construct to 1, not 0. NOTE: this file is not (yet) a row in
// the run-all table - the assert stands as the suite's future seed; wiring it into the gate
// table is a build-affecting edit beyond this row's comment-and-ctor scope.
int main() {
    JE::Action action;
    assert( action.Get_mbr_0x4() == 1 );   // the byte-shape value, cited above
    return 0;
}
