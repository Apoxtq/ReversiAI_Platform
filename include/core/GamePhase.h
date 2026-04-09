#pragma once

#include "core/BitBoard.h"

namespace Reversi {

/**
 * @brief 基准测试阶段枚举
 */
enum class BenchmarkPhase {
    OPENING,    ///< 开局 (0-20回合)
    MIDGAME,    ///< 中局 (21-40回合)
    ENDGAME,    ///< 残局 (41-60回合)
    ALL         ///< 所有阶段
};

} // namespace Reversi
