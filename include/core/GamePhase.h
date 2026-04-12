#pragma once

#include "core/BitBoard.h"

namespace Reversi {

/**
 * @brief Benchmark phase enum
 */
enum class BenchmarkPhase {
    OPENING,    ///< Opening (0-20 moves)
    MIDGAME,    ///< Midgame (21-40 moves)
    ENDGAME,    ///< Endgame (41-60 moves)
    ALL         ///< All phases
};

} // namespace Reversi
