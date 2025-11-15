#ifndef SELECTIONMANAGER_H
#define SELECTIONMANAGER_H

#include <QObject>
#include <QList>

#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>
#include <TopAbs_ShapeEnum.hxx>

class View3D;

enum class SelectionFilterType {
    None,
    Vertex,    // 点
    Edge,      // 边
    Wire,      // 线
    Face,      // 面
    Shell,
    Solid,     // 体
    Compound
};

class SelectionManager : public QObject
{
    Q_OBJECT

public:
    explicit SelectionManager(QObject* parent = nullptr);
    
    void setContext(Handle(AIS_InteractiveContext) context);
    void setView3D(View3D* view) { m_view3D = view; }
    
    // 选择过滤
    void setFilterType(SelectionFilterType type);
    SelectionFilterType getFilterType() const { return m_filterType; }
    
    // 选择操作
    void selectAt(int x, int y);
    void addSelectAt(int x, int y);
    void clearSelection();
    
    // 获取选中的对象
    QList<Handle(AIS_InteractiveObject)> getSelectedObjects() const;
    QList<TopoDS_Shape> getSelectedShapes() const;
    
    // 拾取点
    bool pickPoint(int x, int y, gp_Pnt& point);

signals:
    void selectionChanged();

private:
    Handle(AIS_InteractiveContext) m_context;
    View3D* m_view3D;
    SelectionFilterType m_filterType;
    
    TopAbs_ShapeEnum convertFilterType(SelectionFilterType type);
};

#endif // SELECTIONMANAGER_H

