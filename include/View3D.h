#ifndef VIEW3D_H
#define VIEW3D_H

#include <QWidget>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QContextMenuEvent>
#include <QTime>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <AIS_ViewCube.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

class Document;

class View3D : public QWidget
{
    Q_OBJECT

public:
    explicit View3D(QWidget* parent = nullptr);
    ~View3D();

    void setDocument(Document* doc);
    Document* getDocument() const { return m_document; }
    Handle(AIS_InteractiveContext) getContext() const { return m_context; }
    Handle(V3d_View) getView() const { return m_view; }
    
    // 视图操作
    void fitAll();
    void setViewTop();
    void setViewFront();
    void setViewLeft();
    void setViewRight();
    void setViewBack();
    void setViewBottom();
    void setViewIso();
    
    // 鼠标拾取
    void pickPoint(const QPoint& pos);

protected:
    QPaintEngine* paintEngine() const override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private slots:
    void onContextMenuParameters();
    void onContextMenuColor();
    void onContextMenuTransparency();
    void onContextMenuHide();
    void onContextMenuShowOnly();
    void onContextMenuShowAll();
    void onContextMenuHideAll();
    void onContextMenuClear();

private:
    void init();
    QPoint convertToView(const QPoint& pos);
    void showContextMenu(const QPoint& pos);
    
    Handle(V3d_Viewer) m_viewer;
    Handle(V3d_View) m_view;
    Handle(AIS_InteractiveContext) m_context;
    Handle(AIS_ViewCube) m_viewCube;
    Document* m_document;
    
    // 鼠标交互
    QPoint m_lastMousePos;
    QPoint m_rotationStartPos;
    QPoint m_selectionStartPos;
    bool m_isRotating;
    bool m_isPanning;
    bool m_hasPanned;  // 标记是否进行了平移操作（用于阻止右键菜单）
    bool m_isSelecting;
    QPoint m_pendingMovePos;  // 待处理的鼠标位置（用于节流）
    QTime m_lastUpdateTime;   // 上次更新时间（用于节流）
};

#endif // VIEW3D_H
