#ifndef MODELING_H
#define MODELING_H

#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Vec.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <QList>

class Modeling
{
public:
    // 基本体素
    static TopoDS_Shape createBox(double dx, double dy, double dz);
    static TopoDS_Shape createBox(const gp_Pnt& corner1, const gp_Pnt& corner2);
    
    static TopoDS_Shape createCylinder(double radius, double height);
    static TopoDS_Shape createCylinder(double radius, double height, const gp_Pnt& center, const gp_Dir& axis);
    
    static TopoDS_Shape createSphere(double radius);
    static TopoDS_Shape createSphere(double radius, const gp_Pnt& center);
    
    static TopoDS_Shape createCone(double radius1, double radius2, double height);
    static TopoDS_Shape createCone(double radius1, double radius2, double height, 
                                   const gp_Pnt& center, const gp_Dir& axis);
    
    // 拉伸
    static TopoDS_Shape extrude(const TopoDS_Face& face, double height);
    static TopoDS_Shape extrude(const TopoDS_Face& face, const gp_Dir& direction, double distance);
    static TopoDS_Shape extrude(const TopoDS_Wire& wire, double height);
    static TopoDS_Shape extrude(const TopoDS_Wire& wire, const gp_Dir& direction, double distance);
    
    // 扫略
    static TopoDS_Shape sweep(const TopoDS_Wire& profile, const TopoDS_Wire& path);
    static TopoDS_Shape sweep(const TopoDS_Face& profile, const TopoDS_Wire& path);
    
    // 布尔运算
    static TopoDS_Shape booleanUnion(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2);
    static TopoDS_Shape booleanCut(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2);
    static TopoDS_Shape booleanIntersect(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2);
    
    // 变换
    static TopoDS_Shape translate(const TopoDS_Shape& shape, const gp_Vec& vec);
    static TopoDS_Shape rotate(const TopoDS_Shape& shape, const gp_Ax1& axis, double angle);
    static TopoDS_Shape mirror(const TopoDS_Shape& shape, const gp_Ax2& axis);
    
    // 阵列
    static QList<TopoDS_Shape> linearArray(const TopoDS_Shape& shape, 
                                           const gp_Vec& direction, int count, double spacing);
    static QList<TopoDS_Shape> circularArray(const TopoDS_Shape& shape,
                                            const gp_Pnt& center, const gp_Dir& axis,
                                            int count, double angle);
};

#endif // MODELING_H

