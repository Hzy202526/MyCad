#include "Modeling.h"
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepOffsetAPI_MakePipe.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <gp_Ax2.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax1.hxx>
#include <gp_Pln.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <QList>

TopoDS_Shape Modeling::createBox(double dx, double dy, double dz)
{
    BRepPrimAPI_MakeBox box(dx, dy, dz);
    return box.Shape();
}

TopoDS_Shape Modeling::createBox(const gp_Pnt& corner1, const gp_Pnt& corner2)
{
    gp_Vec vec(corner1, corner2);
    double dx = vec.X();
    double dy = vec.Y();
    double dz = vec.Z();
    
    // 确保尺寸为正
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    if (dz < 0) dz = -dz;
    
    gp_Pnt origin(corner1.X() < corner2.X() ? corner1.X() : corner2.X(),
                  corner1.Y() < corner2.Y() ? corner1.Y() : corner2.Y(),
                  corner1.Z() < corner2.Z() ? corner1.Z() : corner2.Z());
    
    BRepPrimAPI_MakeBox box(gp_Ax2(origin, gp_Dir(0, 0, 1)), dx, dy, dz);
    return box.Shape();
}

TopoDS_Shape Modeling::createCylinder(double radius, double height)
{
    BRepPrimAPI_MakeCylinder cylinder(radius, height);
    return cylinder.Shape();
}

TopoDS_Shape Modeling::createCylinder(double radius, double height, const gp_Pnt& center, const gp_Dir& axis)
{
    gp_Ax2 axis2(center, axis);
    BRepPrimAPI_MakeCylinder cylinder(axis2, radius, height);
    return cylinder.Shape();
}

TopoDS_Shape Modeling::createSphere(double radius)
{
    BRepPrimAPI_MakeSphere sphere(radius);
    return sphere.Shape();
}

TopoDS_Shape Modeling::createSphere(double radius, const gp_Pnt& center)
{
    BRepPrimAPI_MakeSphere sphere(center, radius);
    return sphere.Shape();
}

TopoDS_Shape Modeling::createCone(double radius1, double radius2, double height)
{
    BRepPrimAPI_MakeCone cone(radius1, radius2, height);
    return cone.Shape();
}

TopoDS_Shape Modeling::createCone(double radius1, double radius2, double height, 
                                 const gp_Pnt& center, const gp_Dir& axis)
{
    gp_Ax2 axis2(center, axis);
    BRepPrimAPI_MakeCone cone(axis2, radius1, radius2, height);
    return cone.Shape();
}

TopoDS_Shape Modeling::extrude(const TopoDS_Face& face, double height)
{
    gp_Vec direction(0, 0, height);
    BRepPrimAPI_MakePrism extrude(face, direction);
    return extrude.Shape();
}

TopoDS_Shape Modeling::extrude(const TopoDS_Face& face, const gp_Dir& direction, double distance)
{
    gp_Vec vec(direction);
    vec.Scale(distance);
    BRepPrimAPI_MakePrism extrude(face, vec);
    return extrude.Shape();
}

TopoDS_Shape Modeling::extrude(const TopoDS_Wire& wire, double height)
{
    // 先创建面，再拉伸
    BRepBuilderAPI_MakeFace faceMaker(wire);
    if (!faceMaker.IsDone()) {
        return TopoDS_Shape();
    }
    return extrude(faceMaker.Face(), height);
}

TopoDS_Shape Modeling::extrude(const TopoDS_Wire& wire, const gp_Dir& direction, double distance)
{
    BRepBuilderAPI_MakeFace faceMaker(wire);
    if (!faceMaker.IsDone()) {
        return TopoDS_Shape();
    }
    return extrude(faceMaker.Face(), direction, distance);
}

TopoDS_Shape Modeling::sweep(const TopoDS_Wire& profile, const TopoDS_Wire& path)
{
    BRepOffsetAPI_MakePipe pipe(path, profile);
    if (pipe.IsDone()) {
        return pipe.Shape();
    }
    return TopoDS_Shape();
}

TopoDS_Shape Modeling::sweep(const TopoDS_Face& profile, const TopoDS_Wire& path)
{
    // 从面提取线框
    TopExp_Explorer exp(profile, TopAbs_WIRE);
    if (exp.More()) {
        TopoDS_Wire wire = TopoDS::Wire(exp.Current());
        return sweep(wire, path);
    }
    return TopoDS_Shape();
}

TopoDS_Shape Modeling::booleanUnion(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2)
{
    BRepAlgoAPI_Fuse fuse(shape1, shape2);
    if (fuse.IsDone()) {
        return fuse.Shape();
    }
    return TopoDS_Shape();
}

TopoDS_Shape Modeling::booleanCut(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2)
{
    BRepAlgoAPI_Cut cut(shape1, shape2);
    if (cut.IsDone()) {
        return cut.Shape();
    }
    return TopoDS_Shape();
}

TopoDS_Shape Modeling::booleanIntersect(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2)
{
    BRepAlgoAPI_Common common(shape1, shape2);
    if (common.IsDone()) {
        return common.Shape();
    }
    return TopoDS_Shape();
}

TopoDS_Shape Modeling::translate(const TopoDS_Shape& shape, const gp_Vec& vec)
{
    gp_Trsf trsf;
    trsf.SetTranslation(vec);
    BRepBuilderAPI_Transform transform(shape, trsf);
    return transform.Shape();
}

TopoDS_Shape Modeling::rotate(const TopoDS_Shape& shape, const gp_Ax1& axis, double angle)
{
    gp_Trsf trsf;
    trsf.SetRotation(axis, angle);
    BRepBuilderAPI_Transform transform(shape, trsf);
    return transform.Shape();
}

TopoDS_Shape Modeling::mirror(const TopoDS_Shape& shape, const gp_Ax2& axis)
{
    gp_Trsf trsf;
    trsf.SetMirror(axis);
    BRepBuilderAPI_Transform transform(shape, trsf);
    return transform.Shape();
}

QList<TopoDS_Shape> Modeling::linearArray(const TopoDS_Shape& shape, 
                                          const gp_Vec& direction, int count, double spacing)
{
    QList<TopoDS_Shape> result;
    
    for (int i = 0; i < count; ++i) {
        gp_Vec offset = direction;
        offset.Scale(i * spacing);
        TopoDS_Shape translated = Modeling::translate(shape, offset);
        result.append(translated);
    }
    
    return result;
}

QList<TopoDS_Shape> Modeling::circularArray(const TopoDS_Shape& shape,
                                            const gp_Pnt& center, const gp_Dir& axis,
                                            int count, double angle)
{
    QList<TopoDS_Shape> result;
    
    gp_Ax1 rotationAxis(center, axis);
    double angleStep = angle / count;
    
    for (int i = 0; i < count; ++i) {
        TopoDS_Shape rotated = Modeling::rotate(shape, rotationAxis, i * angleStep);
        result.append(rotated);
    }
    
    return result;
}

