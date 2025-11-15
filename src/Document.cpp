#include "Document.h"
#include "View3D.h"
#include <AIS_Shape.hxx>
#include <Prs3d_Drawer.hxx>
#include <QFile>
#include <QDataStream>
#include <QFileInfo>
#include <QDebug>
#include <QTimer>
#include <BRep_Builder.hxx>
#include <TopoDS_Compound.hxx>
#include <BRepTools.hxx>
#include <Standard_Failure.hxx>
#include <exception>

Document::Document(QObject* parent)
    : QObject(parent)
    , m_view3D(nullptr)
    , m_nextId(1)
{
}

Document::~Document()
{
    clear();
}

void Document::clear()
{
    if (m_view3D && !m_view3D->getContext().IsNull()) {
        Handle(AIS_InteractiveContext) context = m_view3D->getContext();
        for (auto& aisShape : m_aisShapes) {
            if (!aisShape.IsNull()) {
                context->Remove(aisShape, Standard_False);
            }
        }
        context->UpdateCurrentViewer();
    }
    
    m_shapes.clear();
    m_aisShapes.clear();
    m_shapeNames.clear();
    m_nextId = 1;
    
    emit documentChanged();
}

void Document::addShape(const TopoDS_Shape& shape, const QString& name)
{
    qDebug() << "Document::addShape() - 开始";
    
    if (shape.IsNull()) {
        qWarning() << "Document::addShape() - 形状为空";
        return;
    }
    
    QString shapeName = name.isEmpty() ? generateName() : name;
    qDebug() << "Document::addShape() - 形状名称:" << shapeName;
    
    m_shapes.append(shape);
    m_shapeNames.append(shapeName);
    qDebug() << "Document::addShape() - 形状已添加到列表";
    
    qDebug() << "Document::addShape() - 创建AIS显示对象";
    // 创建AIS显示对象
    Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
    // 使用 Shaded 模式
    aisShape->SetDisplayMode(AIS_Shaded);
    // 启用面的边界线显示，实现着色带边效果
    Handle(Prs3d_Drawer) drawer = aisShape->Attributes();
    if (!drawer.IsNull()) {
        drawer->SetFaceBoundaryDraw(Standard_True);
    }
    m_aisShapes.append(aisShape);
    qDebug() << "Document::addShape() - AIS对象创建完成";
    
    // 添加到视图（参考 occQt.cpp 的实现方式，直接调用 Display()）
    if (m_view3D && !m_view3D->getContext().IsNull()) {
        qDebug() << "Document::addShape() - 显示对象到上下文";
        qDebug() << "Document::addShape() - AIS对象指针:" << (void*)aisShape.get();
        qDebug() << "Document::addShape() - 上下文指针:" << (void*)m_view3D->getContext().get();
        
        // 参考 occQt.cpp 中的实现：直接调用 Display(Standard_True)
        // Standard_True 表示立即更新视图
        try {
            qDebug() << "Document::addShape() - 调用 Display(Standard_True)";
            m_view3D->getContext()->Display(aisShape, Standard_True);
            qDebug() << "Document::addShape() - Display() 成功";
        } catch (const Standard_Failure& e) {
            qWarning() << "Document::addShape() - OpenCascade异常:" << e.GetMessageString();
        } catch (const std::exception& e) {
            qWarning() << "Document::addShape() - 标准异常:" << e.what();
        } catch (...) {
            qWarning() << "Document::addShape() - 未知异常";
        }
    } else {
        qWarning() << "Document::addShape() - View3D或上下文为空";
    }
    
    qDebug() << "Document::addShape() - 发送信号";
    emit shapeAdded(shapeName);
    emit documentChanged();
    qDebug() << "Document::addShape() - 完成";
}

void Document::removeShape(int index)
{
    if (index < 0 || index >= m_shapes.size()) {
        return;
    }
    
    QString name = m_shapeNames[index];
    
    // 从视图移除
    if (m_view3D && !m_view3D->getContext().IsNull() && !m_aisShapes[index].IsNull()) {
        m_view3D->getContext()->Remove(m_aisShapes[index], Standard_False);
        m_view3D->getContext()->UpdateCurrentViewer();
    }
    
    m_shapes.removeAt(index);
    m_aisShapes.removeAt(index);
    m_shapeNames.removeAt(index);
    
    emit shapeRemoved(name);
    emit documentChanged();
}

void Document::removeShape(const QString& name)
{
    int index = m_shapeNames.indexOf(name);
    if (index >= 0) {
        removeShape(index);
    }
}

TopoDS_Shape Document::getShape(int index) const
{
    if (index >= 0 && index < m_shapes.size()) {
        return m_shapes[index];
    }
    return TopoDS_Shape();
}

TopoDS_Shape Document::getShape(const QString& name) const
{
    int index = m_shapeNames.indexOf(name);
    if (index >= 0) {
        return m_shapes[index];
    }
    return TopoDS_Shape();
}

Handle(AIS_Shape) Document::getAISShape(int index) const
{
    if (index >= 0 && index < m_aisShapes.size()) {
        return m_aisShapes[index];
    }
    return Handle(AIS_Shape)();
}

Handle(AIS_Shape) Document::getAISShape(const QString& name) const
{
    int index = m_shapeNames.indexOf(name);
    if (index >= 0) {
        return m_aisShapes[index];
    }
    return Handle(AIS_Shape)();
}

int Document::findShapeIndex(Handle(AIS_Shape) aisShape) const
{
    if (aisShape.IsNull()) {
        return -1;
    }
    
    for (int i = 0; i < m_aisShapes.size(); ++i) {
        if (!m_aisShapes[i].IsNull() && m_aisShapes[i] == aisShape) {
            return i;
        }
    }
    
    return -1;
}

bool Document::saveToFile(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QDataStream out(&file);
    
    // 写入版本号
    out << quint32(1);
    
    // 写入形状数量
    out << quint32(m_shapes.size());
    
    // 写入每个形状
    for (int i = 0; i < m_shapes.size(); ++i) {
        // 写入名称
        out << m_shapeNames[i];
        
        // 将形状保存为BREP格式（二进制）
        std::ostringstream oss;
        BRepTools::Write(m_shapes[i], oss);
        QString brepData = QString::fromStdString(oss.str());
        out << brepData;
    }
    
    file.close();
    return true;
}

bool Document::loadFromFile(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QDataStream in(&file);
    
    // 读取版本号
    quint32 version;
    in >> version;
    
    if (version != 1) {
        file.close();
        return false;
    }
    
    clear();
    
    // 读取形状数量
    quint32 count;
    in >> count;
    
    // 读取每个形状
    for (quint32 i = 0; i < count; ++i) {
        QString name;
        in >> name;
        
        QString brepData;
        in >> brepData;
        
        // 从BREP数据恢复形状
        std::istringstream iss(brepData.toStdString());
        TopoDS_Shape shape;
        BRep_Builder builder;
        BRepTools::Read(shape, iss, builder);
        if (!shape.IsNull()) {
            addShape(shape, name);
        }
    }
    
    file.close();
    
    if (m_view3D) {
        m_view3D->fitAll();
    }
    
    return true;
}

QStringList Document::getShapeNames() const
{
    return m_shapeNames;
}

QString Document::generateName(const QString& prefix)
{
    QString name = QString("%1_%2").arg(prefix).arg(m_nextId++);
    return name;
}

