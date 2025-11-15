#ifndef ENCODINGHELPER_H
#define ENCODINGHELPER_H

#include <QString>

// 编码辅助宏：用于GB2312编码的源文件
// 如果源文件是GB2312编码，使用此宏可以确保中文字符串正确显示
#ifdef _WIN32
    // Windows上，如果源文件是GB2312，直接使用字符串字面量
    // Qt会自动使用系统本地编码（GBK）来显示
    #define GB2312_STR(str) QString::fromLocal8Bit(str)
#else
    #define GB2312_STR(str) QString::fromUtf8(str)
#endif

#endif // ENCODINGHELPER_H

