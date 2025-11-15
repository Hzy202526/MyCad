#include "MainWindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QTextCodec>
#include <QDebug>
#include <QLoggingCategory>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMessageBox>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

// 自定义消息处理器，同时输出到控制台和文件
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString logMsg;
    QTextStream stream(&logMsg);
    
    // 格式化时间
    stream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << " ";
    
    // 消息类型
    switch (type) {
    case QtDebugMsg:
        stream << "[DEBUG] ";
        break;
    case QtInfoMsg:
        stream << "[INFO] ";
        break;
    case QtWarningMsg:
        stream << "[WARN] ";
        break;
    case QtCriticalMsg:
        stream << "[CRITICAL] ";
        break;
    case QtFatalMsg:
        stream << "[FATAL] ";
        break;
    }
    
    // 消息内容
    stream << msg;
    
    // 如果有上下文信息，也输出
    if (context.file) {
        stream << " (" << context.file << ":" << context.line << ")";
    }
    
    // 输出到控制台（如果存在）
    QTextStream console(stdout);
    console << logMsg << Qt::endl;
    console.flush();
    
    // 输出到文件
    static QFile logFile("mycad_debug.log");
    if (!logFile.isOpen()) {
        logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    }
    if (logFile.isOpen()) {
        QTextStream fileStream(&logFile);
        fileStream << logMsg << Qt::endl;
        fileStream.flush();
    }
    
    // 如果是致命错误，也显示消息框
    if (type == QtFatalMsg) {
        QMessageBox::critical(nullptr, "Fatal Error", msg);
    }
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    // 在Windows上分配控制台窗口
    if (AllocConsole()) {
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
        freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
        freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
        
        // 设置控制台代码页为UTF-8
        SetConsoleOutputCP(65001);
        SetConsoleCP(65001);
        
        // 设置控制台标题
        SetConsoleTitleA("MyCad Debug Console");
    }
#endif
    
    // 安装自定义消息处理器
    qInstallMessageHandler(messageHandler);
    
    qDebug() << "=== MyCad 启动 ===";
    qDebug() << "开始初始化应用程序...";
    
    QApplication app(argc, argv);
    
    // 设置Qt使用UTF-8编码（源文件需要是UTF-8编码）
    #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    if (codec) {
        QTextCodec::setCodecForLocale(codec);
    }
    #endif
    
    qDebug() << "Qt应用程序对象创建完成";
    
    // 设置应用信息
    app.setApplicationName("MyCad");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("MyCad");
    
    qDebug() << "设置应用信息完成";
    
    // 设置样式
    app.setStyle(QStyleFactory::create("Fusion"));
    qDebug() << "设置样式完成";
    
    qDebug() << "创建主窗口...";
    MainWindow window;
    qDebug() << "主窗口创建完成";
    
    qDebug() << "显示主窗口...";
    window.show();
    qDebug() << "主窗口已显示，进入事件循环";
    
    return app.exec();
}

