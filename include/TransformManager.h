#ifndef TRANSFORMMANAGER_H
#define TRANSFORMMANAGER_H

#include <QObject>
#include <QList>
#include <TopoDS_Shape.hxx>
#include <AIS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>

class Document;

class TransformManager : public QObject
{
    Q_OBJECT

public:
    explicit TransformManager(QObject* parent = nullptr);
    
    void setDocument(Document* doc) { m_document = doc; }
    
    // 布尔运算
    bool booleanUnion(const QList<TopoDS_Shape>& shapes, const QList<Handle(AIS_Shape)>& aisShapes);
    bool booleanCut(const QList<TopoDS_Shape>& shapes, const QList<Handle(AIS_Shape)>& aisShapes);
    bool booleanIntersect(const QList<TopoDS_Shape>& shapes, const QList<Handle(AIS_Shape)>& aisShapes);
    
    // 变换
    bool translate(const QList<TopoDS_Shape>& shapes, const gp_Vec& vec);
    bool rotate(const QList<TopoDS_Shape>& shapes, const gp_Ax1& axis, double angle);
    bool mirror(const QList<TopoDS_Shape>& shapes, const gp_Ax2& axis);
    
    // 阵列
    bool linearArray(const TopoDS_Shape& shape, const gp_Vec& direction, int count, double spacing);
    bool circularArray(const TopoDS_Shape& shape, const gp_Pnt& center, const gp_Dir& axis, int count, double angle);

signals:
    void transformCompleted();

private:
    Document* m_document;
};

#endif // TRANSFORMMANAGER_H

