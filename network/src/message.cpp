/*
    ReversiAI_Platform - Network Module

    @file message.cpp
    @brief Message type implementations
    @date 2026
    @author Project Team
    @license GPL-3.0

    Reference: Egaroucid gtp_command.hpp
*/

#include "network/message.hpp"

namespace Network {

QString coordToGTP(int row, int col) {
    // Egaroucid坐标系转换 (gtp_command.hpp 78-83行)
    // 0-7行列转换为 A-H, 1-8 坐标系
    if (row < 0 || row >= 8 || col < 0 || col >= 8) {
        return QString("PASS");
    }
    
    const QString x_coords = "ABCDEFGH";
    char x = x_coords[col].toLatin1();
    int y = row + 1;
    
    return QString("%1%2").arg(x).arg(y);
}

bool gtpToCoord(const QString& coord, int& row, int& col) {
    // GTP坐标转换 (gtp_command.hpp 参考)
    QString c = coord.trimmed().toUpper();
    
    if (c == "PASS") {
        row = -1;
        col = -1;
        return true;
    }
    
    if (c.length() < 2 || c.length() > 3) {
        return false;
    }
    
    char x_char = c[0].toLatin1();
    int x = x_char - 'A';
    int y = c.mid(1).toInt() - 1;
    
    if (x < 0 || x >= 8 || y < 0 || y >= 8) {
        return false;
    }
    
    row = y;
    col = x;
    return true;
}

bool parseGTPMove(const QString& command, QString& player, int& row, int& col) {
    // 解析 GTP move 命令
    // 格式: "play black D5" 或 "white E6"
    
    QStringList parts = command.trimmed().split(" ", Qt::SkipEmptyParts);
    
    if (parts.size() < 2) {
        return false;
    }
    
    // 解析玩家
    QString playerPart;
    QString coordPart;
    
    if (parts[0] == "play" && parts.size() >= 3) {
        playerPart = parts[1];
        coordPart = parts[2];
    } else {
        playerPart = parts[0];
        coordPart = parts[1];
    }
    
    // 验证玩家颜色
    playerPart = playerPart.toLower();
    if (playerPart != "black" && playerPart != "white") {
        return false;
    }
    
    player = playerPart;
    
    // 转换坐标
    return gtpToCoord(coordPart, row, col);
}

} // namespace Network

