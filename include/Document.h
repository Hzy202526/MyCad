#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>
#include <QString>
#include <QList>

#include <TopoDS_Shape.hxx>
#include <AIS_Shape.hxx>
#include <TCollection_AsciiString.hxx>

class View3D;

// 文档对象，管理所有3D对象
class Document : public QObject
{
    Q_OBJECT

public:
    explicit Document(QObject* parent = nullptr);
    ~Document();
    
    void clear();
    bool isEmpty() const { return m_shapes.isEmpty(); }
    
    // 添加/移除形状
    void addShape(const TopoDS_Shape& shape, const QString& name = QString());
    void removeShape(int index);
    void removeShape(const QString& name);
    int getShapeCount() const { return m_shapes.size(); }
    
    // 获取形状
    TopoDS_Shape getShape(int index) const;
    TopoDS_Shape getShape(const QString& name) const;
    Handle(AIS_Shape) getAISShape(int index) const;
    Handle(AIS_Shape) getAISShape(const QString& name) const;
    
    // 根据AIS对象查找索引
    int findShapeIndex(Handle(AIS_Shape) aisShape) const;
    
    // 序列化
    bool saveToFile(const QString& filename);
    bool loadFromFile(const QString& filename);
    
    // 获取所有形状名称
    QStringList getShapeNames() const;
    
    void setView3D(View3D* view) { m_view3D = view; }

signals:
    void shapeAdded(const QString& name);
    void shapeRemoved(const QString& name);
    void documentChanged();

private:
    QList<TopoDS_Shape> m_shapes;
    QList<Handle(AIS_Shape)> m_aisShapes;
    QStringList m_shapeNames;
    View3D* m_view3D;
    
    int m_nextId;
    QString generateName(const QString& prefix = "Shape");
};

#endif // DOCUMENT_H

