#include "SelectionManager.h"
#include "View3D.h"
#include "Document.h"
#include <AIS_InteractiveObject.hxx>
#include <AIS_Shape.hxx>
#include <SelectMgr_SelectionManager.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <NCollection_Vec2.hxx>
#include <AIS_SelectionScheme.hxx>
#include <QDebug>

SelectionManager::SelectionManager(QObject* parent)
    : QObject(parent)
    , m_view3D(nullptr)
    , m_filterType(SelectionFilterType::None)
{
}

void SelectionManager::setContext(Handle(AIS_InteractiveContext) context)
{
    m_context = context;
}

void SelectionManager::setFilterType(SelectionFilterType type)
{
    m_filterType = type;
    
    // 总是从 View3D 获取最新的上下文（因为 View3D 的 init() 是延迟执行的）
    if (m_view3D == nullptr) {
        qDebug() << "SelectionManager::setFilterType - View3D 为空";
        return;
    }
    
    Handle(AIS_InteractiveContext) context = m_view3D->getContext();
    if (context.IsNull()) {
        qDebug() << "SelectionManager::setFilterType - View3D 的上下文为空，可能还未初始化";
        return;
    }
    
    // 更新上下文引用
    m_context = context;
    
    if (m_view3D->getDocument() == nullptr) {
        qDebug() << "SelectionManager::setFilterType - Document 为空";
        return;
    }
    
    // 清除所有现有过滤器
    m_context->RemoveFilters();
    
    // 根据过滤类型设置选择模式
    // 在 OpenCascade 中，AIS_Shape 的选择模式值需要使用 AIS_Shape::SelectionType() 方法
    // 该方法接受 TopAbs_ShapeEnum 参数并返回对应的选择模式值
    
    Standard_Integer selectionMode = 0;
    
    if (type == SelectionFilterType::None) {
        // 无过滤：使用整个对象的选择模式
        selectionMode = AIS_Shape::SelectionType(TopAbs_SHAPE);
    } else {
        TopAbs_ShapeEnum shapeType = convertFilterType(type);
        // 使用 AIS_Shape::SelectionType() 方法获取正确的选择模式值
        selectionMode = AIS_Shape::SelectionType(shapeType);
    }
    
    qDebug() << "SelectionManager::setFilterType - 过滤类型:" << static_cast<int>(type) << "选择模式:" << selectionMode;
    
    // 为所有已显示的对象设置选择模式
    Document* doc = m_view3D->getDocument();
    int shapeCount = doc->getShapeCount();
    qDebug() << "SelectionManager::setFilterType - 对象数量:" << shapeCount;
    
    for (int i = 0; i < shapeCount; ++i) {
        Handle(AIS_Shape) shape = doc->getAISShape(i);
        if (!shape.IsNull()) {
            // 禁用所有可能的选择模式（0-10 应该足够覆盖所有模式）
            for (Standard_Integer mode = 0; mode <= 10; ++mode) {
                m_context->Deactivate(shape, mode);
            }
            // 激活指定的选择模式
            m_context->Activate(shape, selectionMode);
            qDebug() << "SelectionManager::setFilterType - 为对象" << i << "设置选择模式" << selectionMode;
        }
    }
    
    // 更新视图以反映选择模式的变化
    m_context->UpdateCurrentViewer();
    qDebug() << "SelectionManager::setFilterType - 完成";
}

void SelectionManager::selectAt(int x, int y)
{
    if (m_context.IsNull() || m_view3D == nullptr) {
        return;
    }
    
    // 转换坐标
    QPoint viewPos(x, m_view3D->height() - y);
    
    // 根据过滤类型选择
    // 使用SelectRectangle进行点选（创建一个很小的矩形）
    // SelectRectangle需要NCollection_Vec2<Standard_Integer>类型
    Standard_Integer tolerance = 2;
    NCollection_Vec2<Standard_Integer> p1(viewPos.x() - tolerance, viewPos.y() - tolerance);
    NCollection_Vec2<Standard_Integer> p2(viewPos.x() + tolerance, viewPos.y() + tolerance);
    m_context->SelectRectangle(p1, p2, m_view3D->getView(), AIS_SelectionScheme_Replace);
    
    emit selectionChanged();
}

void SelectionManager::addSelectAt(int x, int y)
{
    if (m_context.IsNull() || m_view3D == nullptr) {
        return;
    }
    
    QPoint viewPos(x, m_view3D->height() - y);
    
    // OpenCascade的AddSelect()需要先调用MoveTo设置鼠标位置
    m_context->MoveTo(viewPos.x(), viewPos.y(), m_view3D->getView(), Standard_True);
    // 使用ShiftSelect进行多选
    m_context->ShiftSelect(Standard_True);
    
    emit selectionChanged();
}

void SelectionManager::clearSelection()
{
    if (m_context.IsNull()) {
        return;
    }
    
    m_context->ClearSelected(Standard_False);
    m_context->UpdateCurrentViewer();
    emit selectionChanged();
}

QList<Handle(AIS_InteractiveObject)> SelectionManager::getSelectedObjects() const
{
    QList<Handle(AIS_InteractiveObject)> result;
    
    if (m_context.IsNull()) {
        return result;
    }
    
    m_context->InitSelected();
    while (m_context->MoreSelected()) {
        Handle(AIS_InteractiveObject) obj = m_context->SelectedInteractive();
        if (!obj.IsNull()) {
            result.append(obj);
        }
        m_context->NextSelected();
    }
    
    return result;
}

QList<TopoDS_Shape> SelectionManager::getSelectedShapes() const
{
    QList<TopoDS_Shape> result;
    
    if (m_context.IsNull()) {
        return result;
    }
    
    m_context->InitSelected();
    while (m_context->MoreSelected()) {
        Handle(AIS_Shape) aisShape = Handle(AIS_Shape)::DownCast(m_context->SelectedInteractive());
        if (!aisShape.IsNull()) {
            result.append(aisShape->Shape());
        }
        m_context->NextSelected();
    }
    
    return result;
}

bool SelectionManager::pickPoint(int x, int y, gp_Pnt& point)
{
    if (m_context.IsNull() || m_view3D == nullptr) {
        return false;
    }
    
    Handle(V3d_View) view = m_view3D->getView();
    if (view.IsNull()) {
        return false;
    }
    
    // 转换坐标
    QPoint viewPos(x, m_view3D->height() - y);
    
    // 拾取点
    Standard_Real X, Y, Z;
    view->Convert(viewPos.x(), viewPos.y(), X, Y, Z);
    point = gp_Pnt(X, Y, Z);
    return true;
}

TopAbs_ShapeEnum SelectionManager::convertFilterType(SelectionFilterType type)
{
    switch (type) {
        case SelectionFilterType::Vertex:
            return TopAbs_VERTEX;
        case SelectionFilterType::Edge:
            return TopAbs_EDGE;
        case SelectionFilterType::Wire:
            return TopAbs_WIRE;
        case SelectionFilterType::Face:
            return TopAbs_FACE;
        case SelectionFilterType::Shell:
            return TopAbs_SHELL;
        case SelectionFilterType::Solid:
            return TopAbs_SOLID;
        case SelectionFilterType::Compound:
            return TopAbs_COMPOUND;
        default:
            return TopAbs_SHAPE;
    }
}
