#pragma once

#include <iosfwd>

namespace chess {

void run_uci_loop();
void run_uci_loop(std::istream& input, std::ostream& output);

}
