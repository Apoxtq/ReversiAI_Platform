#include "mainwindow.h"
#include "ui/MenuWindow.h"
#include <QApplication>
#include <QMessageBox>

// 测试BitBoard功能是否正常
void testBitBoard()
{
    // 这里可以添加简单的BitBoard测试
    // 确保Qt版本能正确链接到我们的核心库
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置应用程序信息
    a.setApplicationName("ReversiAI_Platform");
    a.setApplicationVersion("0.4.0");
    a.setOrganizationName("University of Liverpool");

    try {
        // 测试核心功能
        testBitBoard();

        // 创建目录界面
        MenuWindow menu;
        menu.setWindowTitle("ReversiAI_Platform v0.4.0");
        menu.show();

        return a.exec();
    }
    catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "启动错误",
            QString("应用程序启动失败:\n%1").arg(e.what()));
        return 1;
    }
    catch (...) {
        QMessageBox::critical(nullptr, "启动错误",
            "应用程序启动失败: 未知错误");
        return 1;
    }
}
