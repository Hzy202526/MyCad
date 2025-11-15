#include "FileIO.h"
#include <STEPControl_Reader.hxx>
#include <STEPControl_Writer.hxx>
#include <IGESControl_Reader.hxx>
#include <IGESControl_Writer.hxx>
#include <StlAPI_Reader.hxx>
#include <StlAPI_Writer.hxx>
#include <QFileInfo>
#include <QDebug>

bool FileIO::importFile(const QString& filename, TopoDS_Shape& shape)
{
    QFileInfo fileInfo(filename);
    QString suffix = fileInfo.suffix().toLower();
    
    if (suffix == "step" || suffix == "stp") {
        return importSTEP(filename, shape);
    } else if (suffix == "iges" || suffix == "igs") {
        return importIGES(filename, shape);
    } else if (suffix == "stl") {
        return importSTL(filename, shape);
    } else if (suffix == "obj") {
        return importOBJ(filename, shape);
    } else if (suffix == "gltf") {
        return importGLTF(filename, shape);
    } else if (suffix == "glb") {
        return importGLB(filename, shape);
    }
    
    return false;
}

bool FileIO::exportFile(const QString& filename, const TopoDS_Shape& shape)
{
    QFileInfo fileInfo(filename);
    QString suffix = fileInfo.suffix().toLower();
    
    if (suffix == "step" || suffix == "stp") {
        return exportSTEP(filename, shape);
    } else if (suffix == "iges" || suffix == "igs") {
        return exportIGES(filename, shape);
    } else if (suffix == "stl") {
        return exportSTL(filename, shape);
    } else if (suffix == "obj") {
        return exportOBJ(filename, shape);
    } else if (suffix == "gltf") {
        return exportGLTF(filename, shape);
    } else if (suffix == "glb") {
        return exportGLB(filename, shape);
    }
    
    return false;
}

QStringList FileIO::getImportFormats()
{
    return QStringList() << "STEP (*.step *.stp)"
                        << "IGES (*.iges *.igs)"
                        << "STL (*.stl)"
                        << "OBJ (*.obj)"
                        << "GLTF (*.gltf)"
                        << "GLB (*.glb)";
}

QStringList FileIO::getExportFormats()
{
    return QStringList() << "STEP (*.step *.stp)"
                        << "IGES (*.iges *.igs)"
                        << "STL (*.stl)"
                        << "OBJ (*.obj)"
                        << "GLTF (*.gltf)"
                        << "GLB (*.glb)";
}

QString FileIO::getFileFilter(bool isImport)
{
    QStringList formats = isImport ? getImportFormats() : getExportFormats();
    QString allFormats = formats.join(";;");
    return allFormats + ";;所有文件 (*.*)";
}

bool FileIO::importSTEP(const QString& filename, TopoDS_Shape& shape)
{
    STEPControl_Reader reader;
    IFSelect_ReturnStatus status = reader.ReadFile(filename.toStdString().c_str());
    
    if (status != IFSelect_RetDone) {
        return false;
    }
    
    // 读取所有根对象
    Standard_Integer nbRoots = reader.NbRootsForTransfer();
    if (nbRoots == 0) {
        return false;
    }
    
    // 转移第一个根对象
    reader.TransferRoot(1);
    Standard_Integer nbShapes = reader.NbShapes();
    if (nbShapes == 0) {
        return false;
    }
    
    shape = reader.Shape(1);
    return !shape.IsNull();
}

bool FileIO::importIGES(const QString& filename, TopoDS_Shape& shape)
{
    IGESControl_Reader reader;
    IFSelect_ReturnStatus status = reader.ReadFile(filename.toStdString().c_str());
    
    if (status != IFSelect_RetDone) {
        return false;
    }
    
    reader.TransferRoots();
    Standard_Integer nbShapes = reader.NbShapes();
    if (nbShapes == 0) {
        return false;
    }
    
    shape = reader.Shape(1);
    return !shape.IsNull();
}

bool FileIO::importSTL(const QString& filename, TopoDS_Shape& shape)
{
    StlAPI_Reader reader;
    return reader.Read(shape, filename.toStdString().c_str()) == Standard_True;
}

bool FileIO::importOBJ(const QString& filename, TopoDS_Shape& shape)
{
    // OpenCascade不直接支持OBJ，需要第三方库或手动解析
    // 这里返回false，可以后续扩展
    qDebug() << "OBJ import not yet implemented";
    return false;
}

bool FileIO::importGLTF(const QString& filename, TopoDS_Shape& shape)
{
    // OpenCascade不直接支持GLTF，需要第三方库
    qDebug() << "GLTF import not yet implemented";
    return false;
}

bool FileIO::importGLB(const QString& filename, TopoDS_Shape& shape)
{
    // GLB是GLTF的二进制格式
    qDebug() << "GLB import not yet implemented";
    return false;
}

bool FileIO::exportSTEP(const QString& filename, const TopoDS_Shape& shape)
{
    STEPControl_Writer writer;
    IFSelect_ReturnStatus status = writer.Transfer(shape, STEPControl_AsIs);
    
    if (status != IFSelect_RetDone) {
        return false;
    }
    
    return writer.Write(filename.toStdString().c_str()) == IFSelect_RetDone;
}

bool FileIO::exportIGES(const QString& filename, const TopoDS_Shape& shape)
{
    IGESControl_Writer writer;
    writer.AddShape(shape);
    return writer.Write(filename.toStdString().c_str()) == Standard_True;
}

bool FileIO::exportSTL(const QString& filename, const TopoDS_Shape& shape)
{
    StlAPI_Writer writer;
    return writer.Write(shape, filename.toStdString().c_str()) == Standard_True;
}

bool FileIO::exportOBJ(const QString& filename, const TopoDS_Shape& shape)
{
    // OpenCascade不直接支持OBJ导出
    qDebug() << "OBJ export not yet implemented";
    return false;
}

bool FileIO::exportGLTF(const QString& filename, const TopoDS_Shape& shape)
{
    qDebug() << "GLTF export not yet implemented";
    return false;
}

bool FileIO::exportGLB(const QString& filename, const TopoDS_Shape& shape)
{
    qDebug() << "GLB export not yet implemented";
    return false;
}

