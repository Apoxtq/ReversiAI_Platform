#include "mainwindow.h"
#include "ui/MenuWindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <Windows.h>
#include <DbgHelp.h>
#include <iostream>

#pragma comment(lib, "dbgeng.lib")

// 日志文件路径
static QString g_logFilePath;

// 自定义消息处理器
void customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString logMessage;
    
    switch (type) {
        case QtDebugMsg:
            logMessage = QString("[%1] DEBUG: %2").arg(timestamp).arg(msg);
            break;
        case QtInfoMsg:
            logMessage = QString("[%1] INFO: %2").arg(timestamp).arg(msg);
            break;
        case QtWarningMsg:
            logMessage = QString("[%1] WARNING: %2").arg(timestamp).arg(msg);
            break;
        case QtCriticalMsg:
            logMessage = QString("[%1] CRITICAL: %2").arg(timestamp).arg(msg);
            break;
        case QtFatalMsg:
            logMessage = QString("[%1] FATAL: %2").arg(timestamp).arg(msg);
            break;
    }
    
    // 输出到控制台
    std::cerr << logMessage.toStdString() << std::endl;
    
    // 输出到日志文件
    QFile logFile(g_logFilePath);
    if (logFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << logMessage << Qt::endl;
        logFile.close();
    }
}

// 全局崩溃处理器
LONG WINAPI crashHandler(EXCEPTION_POINTERS* ExceptionInfo) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    QString crashLogPath = QCoreApplication::applicationDirPath() + 
                          QString("/crash_log_%1.txt").arg(timestamp);
    
    QFile file(crashLogPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "=== CRASH DUMP ===" << Qt::endl;
        out << "Timestamp: " << QDateTime::currentDateTime().toString() << Qt::endl;
        out << "Exception Code: 0x" << QString::number(ExceptionInfo->ExceptionRecord->ExceptionCode, 16) << Qt::endl;
        out << "Exception Address: 0x" << QString::number(reinterpret_cast<quintptr>(ExceptionInfo->ExceptionRecord->ExceptionAddress), 16) << Qt::endl;
        
        CONTEXT* ctx = ExceptionInfo->ContextRecord;
        out << Qt::endl << "Registers:" << Qt::endl;
        out << "RAX: 0x" << QString::number(ctx->Rax, 16) << Qt::endl;
        out << "RBX: 0x" << QString::number(ctx->Rbx, 16) << Qt::endl;
        out << "RCX: 0x" << QString::number(ctx->Rcx, 16) << Qt::endl;
        out << "RDX: 0x" << QString::number(ctx->Rdx, 16) << Qt::endl;
        out << "RSI: 0x" << QString::number(ctx->Rsi, 16) << Qt::endl;
        out << "RDI: 0x" << QString::number(ctx->Rdi, 16) << Qt::endl;
        out << "RSP: 0x" << QString::number(ctx->Rsp, 16) << Qt::endl;
        out << "RBP: 0x" << QString::number(ctx->Rbp, 16) << Qt::endl;
        out << "RIP: 0x" << QString::number(ctx->Rip, 16) << Qt::endl;
        out << Qt::endl;
        
        file.close();
    }
    
    // 也输出到 stderr
    std::cerr << "CRASH: Exception 0x" << std::hex << ExceptionInfo->ExceptionRecord->ExceptionCode 
              << " at 0x" << ExceptionInfo->ExceptionRecord->ExceptionAddress << std::endl;
    
    return EXCEPTION_CONTINUE_SEARCH;
}

// 测试BitBoard功能是否正常
void testBitBoard()
{
    // 这里可以添加简单的BitBoard测试
    // 确保Qt版本能正确链接到我们的核心库
}

int main(int argc, char *argv[])
{
    // 设置崩溃处理器
    SetUnhandledExceptionFilter(crashHandler);
    
    QApplication a(argc, argv);

    // 设置日志文件
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    g_logFilePath = QCoreApplication::applicationDirPath() + 
                    QString("/reversi_debug_%1.log").arg(timestamp);
    
    // 安装自定义消息处理器
    qInstallMessageHandler(customMessageHandler);
    
    qDebug() << "=== Application Starting ===";
    qDebug() << "Log file:" << g_logFilePath;

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
