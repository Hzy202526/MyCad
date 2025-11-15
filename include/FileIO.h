#ifndef FILEIO_H
#define FILEIO_H

#include <QString>
#include <TopoDS_Shape.hxx>

class FileIO
{
public:
    // 导入文件
    static bool importFile(const QString& filename, TopoDS_Shape& shape);
    
    // 导出文件
    static bool exportFile(const QString& filename, const TopoDS_Shape& shape);
    
    // 获取支持的文件格式
    static QStringList getImportFormats();
    static QStringList getExportFormats();
    static QString getFileFilter(bool isImport);
    
private:
    static bool importSTEP(const QString& filename, TopoDS_Shape& shape);
    static bool importIGES(const QString& filename, TopoDS_Shape& shape);
    static bool importSTL(const QString& filename, TopoDS_Shape& shape);
    static bool importOBJ(const QString& filename, TopoDS_Shape& shape);
    static bool importGLTF(const QString& filename, TopoDS_Shape& shape);
    static bool importGLB(const QString& filename, TopoDS_Shape& shape);
    
    static bool exportSTEP(const QString& filename, const TopoDS_Shape& shape);
    static bool exportIGES(const QString& filename, const TopoDS_Shape& shape);
    static bool exportSTL(const QString& filename, const TopoDS_Shape& shape);
    static bool exportOBJ(const QString& filename, const TopoDS_Shape& shape);
    static bool exportGLTF(const QString& filename, const TopoDS_Shape& shape);
    static bool exportGLB(const QString& filename, const TopoDS_Shape& shape);
};

#endif // FILEIO_H

