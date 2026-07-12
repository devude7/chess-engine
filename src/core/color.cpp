#include "chess/core/color.hpp"

namespace chess {

Color opposite(Color color) {
    return color == Color::White ? Color::Black : Color::White;
}

}
