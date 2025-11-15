#include "View3D.h"
#include "Document.h"
#include <QDebug>
#include <QTimer>
#include <QTime>
#include <QMenu>
#include <QColorDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QPointer>
#include <QApplication>
#include <QDialog>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDoubleSpinBox>

#include <OpenGl_GraphicDriver.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Aspect_Handle.hxx>
#include <Aspect_Window.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <AIS_InteractiveObject.hxx>
#include <AIS_Shape.hxx>
#include <AIS_ViewCube.hxx>
#include <Prs3d_Drawer.hxx>
#include <Aspect_TypeOfTriedronPosition.hxx>
#include <Graphic3d_TransformPers.hxx>
#include <AIS_SelectionScheme.hxx>
#include <NCollection_Vec2.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <Quantity_Color.hxx>
#include <Prs3d_ShadingAspect.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <TopAbs_ShapeEnum.hxx>

#ifdef _WIN32
    #ifndef WNT
        #define WNT
    #endif
    #include <WNT_Window.hxx>
#elif defined(__APPLE__) && !defined(MACOSX_USE_GLX)
    #include <Cocoa_Window.hxx>
#else
    #undef Bool
    #undef CursorShape
    #undef None
    #undef KeyPress
    #undef KeyRelease
    #undef FocusIn
    #undef FocusOut
    #undef FontChange
    #undef Expose
    #include <Xw_Window.hxx>
#endif

// 静态图形驱动，全局共享
static Handle(Graphic3d_GraphicDriver)& GetGraphicDriver()
{
    static Handle(Graphic3d_GraphicDriver) aGraphicDriver;
    return aGraphicDriver;
}

View3D::View3D(QWidget* parent)
    : QWidget(parent)
    , m_viewer(nullptr)
    , m_view(nullptr)
    , m_context(nullptr)
    , m_viewCube(nullptr)
    , m_document(nullptr)
    , m_isRotating(false)
    , m_isPanning(false)
    , m_hasPanned(false)
    , m_isSelecting(false)
    , m_rotationStartPos(0, 0)
    , m_selectionStartPos(0, 0)
    , m_pendingMovePos(0, 0)
{
    // 设置属性以支持OpenGL渲染（参考 occView.cpp）
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);
    
    // 设置焦点策略
    setFocusPolicy(Qt::StrongFocus);
    
    // 启用鼠标跟踪
    setMouseTracking(true);
    
    // 初始化（延迟到窗口显示时，确保窗口句柄有效）
    QTimer::singleShot(0, this, [this]() { init(); });
}

View3D::~View3D()
{
}

void View3D::setDocument(Document* doc)
{
    m_document = doc;
    if (m_document) {
        m_document->setView3D(this);
    }
}

void View3D::init()
{
    // 如果已经初始化，直接返回
    if (!m_viewer.IsNull()) {
        return;
    }
    
    // 创建显示连接
    Handle(Aspect_DisplayConnection) aDisplayConnection = new Aspect_DisplayConnection();
    
    // 获取或创建图形驱动（全局共享）
    if (GetGraphicDriver().IsNull()) {
        GetGraphicDriver() = new OpenGl_GraphicDriver(aDisplayConnection);
    }
    
    // 创建查看器（OpenCascade 7.9.2 只接受一个参数）
    m_viewer = new V3d_Viewer(GetGraphicDriver());
    
    // 创建视图
    m_view = m_viewer->CreateView();
    
    // 获取窗口句柄（确保窗口已创建）
    WId window_handle = (WId)winId();
    
    // 根据平台创建相应的窗口（参考 occView.cpp）
    Handle(Aspect_Window) wind;
    #ifdef WNT
        wind = new WNT_Window((Aspect_Handle)window_handle);
    #elif defined(__APPLE__) && !defined(MACOSX_USE_GLX)
        wind = new Cocoa_Window((NSView*)window_handle);
    #else
        wind = new Xw_Window(aDisplayConnection, (Window)window_handle);
    #endif
    
    // 设置窗口
    m_view->SetWindow(wind);
    if (!wind->IsMapped()) {
        wind->Map();
    }
    
    // 创建交互上下文
    m_context = new AIS_InteractiveContext(m_viewer);
    
    // 设置像素容差以提高探测精度（特别是对于边和顶点的探测）
    // 默认值通常是 2-3 像素，增加到 8 像素可以提高探测精度
    m_context->SetPixelTolerance(8);
    
    // 设置默认灯光
    m_viewer->SetDefaultLights();
    m_viewer->SetLightOn();
    
    // 设置背景色
    m_view->SetBackgroundColor(Quantity_NOC_BLACK);
    
    // 调整视图大小
    m_view->MustBeResized();
    
    // 设置显示模式：着色带边显示
    // OpenCascade 7.9.2 使用 Shaded 模式，然后为对象设置带边显示
    m_context->SetDisplayMode(AIS_Shaded, Standard_True);
    // 设置默认绘图属性，启用面边界显示
    Handle(Prs3d_Drawer) drawer = m_context->DefaultDrawer();
    if (!drawer.IsNull()) {
        drawer->SetFaceBoundaryDraw(Standard_True); // 启用面边界显示
    }
    
    // 创建并添加 OpenCascade 自带的 ViewCube（左下角）
    m_viewCube = new AIS_ViewCube();
    
    // 设置 ViewCube 大小
    m_viewCube->SetSize(60); // 设置立方体大小（像素）
    
    // 设置 ViewCube 位置为左下角（使用 TransformPersistence）
    Handle(Graphic3d_TransformPers) transformPers = 
        new Graphic3d_TransformPers(
            Graphic3d_TMF_TriedronPers,
            Aspect_TOTP_LEFT_LOWER,
            Graphic3d_Vec2i(100, 100));
    m_viewCube->SetTransformPersistence(transformPers);
    
    // 启用坐标轴显示
    m_viewCube->SetDrawAxes(Standard_True);
    
    // 将 ViewCube 添加到交互上下文并显示
    m_context->Display(m_viewCube, Standard_False);
    
    // 初始重绘
    m_view->Redraw();
}

QPaintEngine* View3D::paintEngine() const
{
    // 返回nullptr，让OpenCascade直接使用OpenGL渲染
    return nullptr;
}

void View3D::paintEvent(QPaintEvent* /*event*/)
{
    if (m_view.IsNull()) {
        return;
    }
    
    // 参考 occView.cpp：直接调用 Redraw()
    m_view->Redraw();
}

void View3D::resizeEvent(QResizeEvent* /*event*/)
{
    if (m_view.IsNull()) {
        return;
    }
    
    m_view->MustBeResized();
}

void View3D::fitAll()
{
    if (m_view.IsNull()) {
        return;
    }
    
    m_view->FitAll();
    m_view->ZFitAll();
    m_view->Redraw();
}

void View3D::setViewTop()
{
    if (m_view.IsNull()) return;
    m_view->SetProj(V3d_Ypos);
    m_view->FitAll();
    update();
}

void View3D::setViewFront()
{
    if (m_view.IsNull()) return;
    m_view->SetProj(V3d_Zpos);
    m_view->FitAll();
    update();
}

void View3D::setViewLeft()
{
    if (m_view.IsNull()) return;
    m_view->SetProj(V3d_Xneg);
    m_view->FitAll();
    update();
}

void View3D::setViewRight()
{
    if (m_view.IsNull()) return;
    m_view->SetProj(V3d_Xpos);
    m_view->FitAll();
    update();
}

void View3D::setViewBack()
{
    if (m_view.IsNull()) return;
    m_view->SetProj(V3d_Zneg);
    m_view->FitAll();
    update();
}

void View3D::setViewBottom()
{
    if (m_view.IsNull()) return;
    m_view->SetProj(V3d_Yneg);
    m_view->FitAll();
    update();
}

void View3D::setViewIso()
{
    if (m_view.IsNull()) return;
    m_view->SetProj(V3d_XposYnegZpos);
    m_view->FitAll();
    update();
}

void View3D::pickPoint(const QPoint& pos)
{
    if (m_context.IsNull() || m_view.IsNull()) {
        return;
    }
    
    QPoint viewPos = convertToView(pos);
    gp_Pnt point;
    
    // 尝试从视图坐标转换到3D坐标
    Standard_Real X, Y, Z;
    m_view->Convert(viewPos.x(), viewPos.y(), X, Y, Z);
    point = gp_Pnt(X, Y, Z);
    
    // 创建点并添加到文档
    if (m_document) {
        // 这里可以创建一个点标记或添加到文档
        // 暂时只做坐标转换，具体实现可以根据需求扩展
    }
}

void View3D::mousePressEvent(QMouseEvent* event)
{
    if (m_view.IsNull() || m_context.IsNull()) {
        return;
    }
    
    m_lastMousePos = event->pos();
    QPoint viewPos = convertToView(event->pos());
    
    // 右键：显示上下文菜单或平移
    if (event->button() == Qt::RightButton) {
        // 如果点击时没有拖动，显示右键菜单
        // 否则开始平移（在mouseMoveEvent中处理）
        m_isPanning = false; // 先不开始平移，等待鼠标移动
        m_hasPanned = false; // 重置平移标志
        m_lastMousePos = event->pos(); // 记录位置用于判断是否拖动
    }
    // 中键：旋转
    else if (event->button() == Qt::MiddleButton) {
        m_isRotating = true;
        // 使用原始坐标（不转换Y）进行旋转，与Rotation保持一致
        m_rotationStartPos = event->pos();
        // 初始化旋转中心
        m_view->StartRotation(m_rotationStartPos.x(), m_rotationStartPos.y(), 0.4);
        setCursor(Qt::SizeAllCursor);
    }
    // 左键：选择或框选
    else if (event->button() == Qt::LeftButton) {
        // 先更新鼠标位置，检测当前对象
        m_context->MoveTo(viewPos.x(), viewPos.y(), m_view, Standard_True);
        
        // 检查是否点击了ViewCube
        if (!m_viewCube.IsNull()) {
            // 使用OpenCascade的检测机制检查是否选中了ViewCube
            m_context->InitDetected();
            if (m_context->MoreDetected()) {
                Handle(AIS_InteractiveObject) detected = m_context->DetectedInteractive();
                if (!detected.IsNull() && detected == m_viewCube) {
                    // 点击了ViewCube，让ViewCube处理交互
                    m_context->SelectDetected();
                    m_context->UpdateCurrentViewer();
                    update();
                    return;
                }
            }
        }
        
        // 开始选择操作（记录起始位置，实际选择在mouseReleaseEvent中完成）
        m_isSelecting = true;
        m_selectionStartPos = viewPos;
    }
}

void View3D::mouseMoveEvent(QMouseEvent* event)
{
    if (m_view.IsNull() || m_context.IsNull()) {
        return;
    }
    
    QPoint currentPos = event->pos();
    QPoint delta = currentPos - m_lastMousePos;
    QPoint viewPos = convertToView(currentPos);
    
    // 右键拖动：平移
    if ((event->buttons() & Qt::RightButton)) {
        QPoint delta = currentPos - m_lastMousePos;
        if (delta.manhattanLength() > 3) {
            // 开始平移
            if (!m_isPanning) {
                m_isPanning = true;
                m_hasPanned = true; // 标记已进行平移
                setCursor(Qt::ClosedHandCursor);
            }
            // Pan 需要传入增量，注意Y坐标需要取反
            m_view->Pan(delta.x(), -delta.y());
            update();
        }
    }
    // 中键拖动：旋转
    else if (m_isRotating && (event->buttons() & Qt::MiddleButton)) {
        // 只有在鼠标真正移动时才旋转，避免抖动
        if (delta.manhattanLength() > 2) {
            // 使用原始坐标（不转换Y）进行旋转，以修正旋转方向
            // OpenCascade的Rotation方法期望的坐标系统与convertToView转换后的不同
            m_view->Rotation(currentPos.x(), currentPos.y());
            update();
        }
    }
    // 左键拖动：框选
    else if (m_isSelecting && (event->buttons() & Qt::LeftButton)) {
        // 如果移动距离足够大，开始框选
        QPoint deltaSelect = viewPos - m_selectionStartPos;
        if (deltaSelect.manhattanLength() > 5) {
            // 框选模式
            NCollection_Vec2<Standard_Integer> p1(m_selectionStartPos.x(), m_selectionStartPos.y());
            NCollection_Vec2<Standard_Integer> p2(viewPos.x(), viewPos.y());
            
            // 根据是否按下Ctrl决定是添加还是替换选择
            AIS_SelectionScheme scheme = (event->modifiers() & Qt::ControlModifier) 
                ? AIS_SelectionScheme_Add 
                : AIS_SelectionScheme_Replace;
            
            m_context->SelectRectangle(p1, p2, m_view, scheme);
            m_context->UpdateCurrentViewer();
            update();
        } else {
            // 小范围移动，更新高亮显示
            m_context->MoveTo(viewPos.x(), viewPos.y(), m_view, Standard_True);
            if (!m_view.IsNull()) {
                m_view->RedrawImmediate();
            }
        }
    }
    // 普通移动：更新高亮显示（使用节流以提高性能）
    else {
        m_pendingMovePos = viewPos;
        
        // 节流：限制更新频率到约 60fps（每 16ms 更新一次）
        QTime currentTime = QTime::currentTime();
        if (m_lastUpdateTime.isNull() || m_lastUpdateTime.msecsTo(currentTime) >= 16) {
            m_context->MoveTo(m_pendingMovePos.x(), m_pendingMovePos.y(), m_view, Standard_True);
            // 使用 RedrawImmediate 代替 UpdateCurrentViewer + update，更高效
            if (!m_view.IsNull()) {
                m_view->RedrawImmediate();
            }
            m_lastUpdateTime = currentTime;
        }
    }
    
    m_lastMousePos = currentPos;
}

void View3D::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton) {
        // 右键释放时，如果进行了平移，结束平移
        // 右键菜单由 contextMenuEvent 处理，这里不再调用
        m_isPanning = false;
        setCursor(Qt::ArrowCursor);
    }
    else if (event->button() == Qt::MiddleButton) {
        m_isRotating = false;
        setCursor(Qt::ArrowCursor);
    }
    else if (event->button() == Qt::LeftButton) {
        if (m_isSelecting && !m_context.IsNull()) {
            QPoint viewPos = convertToView(event->pos());
            QPoint deltaSelect = viewPos - m_selectionStartPos;
            
            // 更新鼠标位置
            m_context->MoveTo(viewPos.x(), viewPos.y(), m_view, Standard_True);
            
            // 如果移动距离很小，执行点选
            if (deltaSelect.manhattanLength() <= 5) {
                // 点选模式
                if (event->modifiers() & Qt::ControlModifier) {
                    // Ctrl+点击：多选模式（添加或移除选择）
                    m_context->ShiftSelect(Standard_True);
                } else {
                    // 普通点击：单选模式（替换选择）
                    m_context->SelectDetected();
                }
            }
            // 否则框选已经在mouseMoveEvent中完成
            
            m_context->UpdateCurrentViewer();
            update();
        }
        m_isSelecting = false;
    }
}

void View3D::wheelEvent(QWheelEvent* event)
{
    if (m_view.IsNull()) {
        return;
    }
    
    // 中键滚动：缩放
    // 检查是否按下了中键（或者只使用滚轮事件，因为通常滚轮就是中键的滚动）
    double delta = event->angleDelta().y() / 120.0;
    
    QPoint pos = event->pos();
    QPoint viewPos = convertToView(pos);
    
    // 使用Zoom进行缩放
    // 向上滚动（delta > 0）放大，向下滚动（delta < 0）缩小
    double zoomFactor = (delta > 0) ? 1.1 : 0.9;
    
    // 使用StartZoomAtPoint和ZoomAtPoint实现以鼠标位置为中心的缩放
    m_view->StartZoomAtPoint(viewPos.x(), viewPos.y());
    Standard_Integer newX = viewPos.x();
    Standard_Integer newY = viewPos.y() + (Standard_Integer)(delta * 10);
    m_view->ZoomAtPoint(viewPos.x(), viewPos.y(), newX, newY);
    update();
}

void View3D::keyPressEvent(QKeyEvent* event)
{
    if (m_view.IsNull()) {
        return;
    }
    
    switch (event->key()) {
        case Qt::Key_F:
            fitAll();
            break;
        default:
            QWidget::keyPressEvent(event);
    }
}

QPoint View3D::convertToView(const QPoint& pos)
{
    // 转换Qt坐标到OpenCascade视图坐标
    return QPoint(pos.x(), height() - pos.y());
}

void View3D::contextMenuEvent(QContextMenuEvent* event)
{
    // 只有在没有进行平移操作时才显示菜单
    if (!m_hasPanned) {
        showContextMenu(event->globalPos());
    }
    // 重置平移标志
    m_hasPanned = false;
    m_isPanning = false;
}

void View3D::showContextMenu(const QPoint& pos)
{
    if (m_context.IsNull() || m_document == nullptr) {
        return;
    }
    
    QMenu menu(this);
    
    // 获取选中的对象
    QList<Handle(AIS_InteractiveObject)> selectedObjects;
    m_context->InitSelected();
    while (m_context->MoreSelected()) {
        Handle(AIS_InteractiveObject) obj = m_context->SelectedInteractive();
        if (!obj.IsNull()) {
            // 排除ViewCube
            if (obj != m_viewCube) {
                selectedObjects.append(obj);
            }
        }
        m_context->NextSelected();
    }
    
    bool hasSelection = !selectedObjects.isEmpty();
    bool hasObjects = m_document->getShapeCount() > 0;
    
    // 用于存储要执行的操作
    QAction* actionToExecute = nullptr;
    
    // 如果有选中对象，显示所有菜单项
    if (hasSelection) {
        // 参数
        QAction* parametersAction = menu.addAction("参数");
        connect(parametersAction, &QAction::triggered, [&menu, &actionToExecute, parametersAction]() {
            actionToExecute = parametersAction;
            menu.close();  // 立即关闭菜单
        });
        
        menu.addSeparator();
        
        // 颜色
        QAction* colorAction = menu.addAction("颜色");
        connect(colorAction, &QAction::triggered, [&menu, &actionToExecute, colorAction]() {
            actionToExecute = colorAction;
            menu.close();
        });
        
        // 透明度
        QAction* transparencyAction = menu.addAction("透明度");
        connect(transparencyAction, &QAction::triggered, [&menu, &actionToExecute, transparencyAction]() {
            actionToExecute = transparencyAction;
            menu.close();
        });
        
        menu.addSeparator();
        
        // 隐藏
        QAction* hideAction = menu.addAction("隐藏");
        connect(hideAction, &QAction::triggered, [&menu, &actionToExecute, hideAction]() {
            actionToExecute = hideAction;
            menu.close();
        });
        
        // 仅显示
        QAction* showOnlyAction = menu.addAction("仅显示");
        connect(showOnlyAction, &QAction::triggered, [&menu, &actionToExecute, showOnlyAction]() {
            actionToExecute = showOnlyAction;
            menu.close();
        });
        
        // 显示全部
        QAction* showAllAction = menu.addAction("显示全部");
        showAllAction->setEnabled(hasObjects);
        connect(showAllAction, &QAction::triggered, [&menu, &actionToExecute, showAllAction]() {
            actionToExecute = showAllAction;
            menu.close();
        });
        
        // 隐藏全部
        QAction* hideAllAction = menu.addAction("隐藏全部");
        hideAllAction->setEnabled(hasObjects);
        connect(hideAllAction, &QAction::triggered, [&menu, &actionToExecute, hideAllAction]() {
            actionToExecute = hideAllAction;
            menu.close();
        });
        
        menu.addSeparator();
        
        // 清空
        QAction* clearAction = menu.addAction("清空");
        clearAction->setEnabled(hasObjects);
        connect(clearAction, &QAction::triggered, [&menu, &actionToExecute, clearAction]() {
            actionToExecute = clearAction;
            menu.close();
        });
    } else {
        // 如果没有选中对象，显示显示全部、隐藏全部和清空
        if (hasObjects) {
            // 显示全部
            QAction* showAllAction = menu.addAction("显示全部");
            connect(showAllAction, &QAction::triggered, [&menu, &actionToExecute, showAllAction]() {
                actionToExecute = showAllAction;
                menu.close();
            });
            
            // 隐藏全部
            QAction* hideAllAction = menu.addAction("隐藏全部");
            connect(hideAllAction, &QAction::triggered, [&menu, &actionToExecute, hideAllAction]() {
                actionToExecute = hideAllAction;
                menu.close();
            });
            
            menu.addSeparator();
            
            // 清空
            QAction* clearAction = menu.addAction("清空");
            connect(clearAction, &QAction::triggered, [&menu, &actionToExecute, clearAction]() {
                actionToExecute = clearAction;
                menu.close();
            });
        }
    }
    
    // 执行菜单
    menu.exec(pos);
    
    // 确保菜单完全关闭
    menu.hide();
    menu.setVisible(false);
    menu.close();
    
    // 处理事件，确保菜单完全消失
    QApplication::processEvents();
    
    // 执行选中的操作
    if (actionToExecute != nullptr) {
        if (actionToExecute->text() == "参数") {
            onContextMenuParameters();
        } else if (actionToExecute->text() == "颜色") {
            onContextMenuColor();
        } else if (actionToExecute->text() == "透明度") {
            onContextMenuTransparency();
        } else if (actionToExecute->text() == "隐藏") {
            onContextMenuHide();
        } else if (actionToExecute->text() == "仅显示") {
            onContextMenuShowOnly();
        } else if (actionToExecute->text() == "显示全部") {
            onContextMenuShowAll();
        } else if (actionToExecute->text() == "隐藏全部") {
            onContextMenuHideAll();
        } else if (actionToExecute->text() == "清空") {
            onContextMenuClear();
        }
    }
}

void View3D::onContextMenuParameters()
{
    if (m_context.IsNull() || m_document == nullptr) {
        return;
    }
    
    // 获取选中的对象
    QList<Handle(AIS_Shape)> selectedShapes;
    m_context->InitSelected();
    while (m_context->MoreSelected()) {
        Handle(AIS_InteractiveObject) obj = m_context->SelectedInteractive();
        if (!obj.IsNull() && obj != m_viewCube) {
            Handle(AIS_Shape) shape = Handle(AIS_Shape)::DownCast(obj);
            if (!shape.IsNull()) {
                selectedShapes.append(shape);
            }
        }
        m_context->NextSelected();
    }
    
    if (selectedShapes.isEmpty()) {
        return;
    }
    
    // 构建参数信息字符串
    QString info = "选中对象参数信息：\n\n";
    
    for (int i = 0; i < selectedShapes.size(); ++i) {
        const auto& shape = selectedShapes[i];
        TopoDS_Shape topoShape = shape->Shape();
        
        if (topoShape.IsNull()) {
            continue;
        }
        
        info += QString("对象 %1:\n").arg(i + 1);
        
        // 获取形状类型
        TopAbs_ShapeEnum shapeType = topoShape.ShapeType();
        QString typeStr;
        switch (shapeType) {
            case TopAbs_VERTEX: typeStr = "顶点"; break;
            case TopAbs_EDGE: typeStr = "边"; break;
            case TopAbs_WIRE: typeStr = "线框"; break;
            case TopAbs_FACE: typeStr = "面"; break;
            case TopAbs_SHELL: typeStr = "壳体"; break;
            case TopAbs_SOLID: typeStr = "实体"; break;
            case TopAbs_COMPOUND: typeStr = "复合体"; break;
            default: typeStr = "未知"; break;
        }
        info += QString("  类型: %1\n").arg(typeStr);
        
        // 计算几何属性
        GProp_GProps props;
        BRepGProp::LinearProperties(topoShape, props);
        double length = props.Mass();
        
        BRepGProp::SurfaceProperties(topoShape, props);
        double area = props.Mass();
        
        // 如果是实体，计算体积
        if (shapeType == TopAbs_SOLID || shapeType == TopAbs_COMPOUND) {
            BRepGProp::VolumeProperties(topoShape, props);
            double volume = props.Mass();
            info += QString("  体积: %1\n").arg(volume, 0, 'f', 6);
        }
        
        info += QString("  表面积: %1\n").arg(area, 0, 'f', 6);
        info += QString("  长度: %1\n").arg(length, 0, 'f', 6);
        
        // 计算边界框
        Bnd_Box bbox;
        BRepBndLib::Add(topoShape, bbox);
        if (!bbox.IsVoid()) {
            double xMin, yMin, zMin, xMax, yMax, zMax;
            bbox.Get(xMin, yMin, zMin, xMax, yMax, zMax);
            info += QString("  边界框:\n");
            info += QString("    X: [%1, %2]\n").arg(xMin, 0, 'f', 6).arg(xMax, 0, 'f', 6);
            info += QString("    Y: [%1, %2]\n").arg(yMin, 0, 'f', 6).arg(yMax, 0, 'f', 6);
            info += QString("    Z: [%1, %2]\n").arg(zMin, 0, 'f', 6).arg(zMax, 0, 'f', 6);
            
            double dx = xMax - xMin;
            double dy = yMax - yMin;
            double dz = zMax - zMin;
            info += QString("  尺寸: %1 × %2 × %3\n").arg(dx, 0, 'f', 6).arg(dy, 0, 'f', 6).arg(dz, 0, 'f', 6);
        }
        
        // 获取质心
        gp_Pnt center = props.CentreOfMass();
        info += QString("  质心: (%1, %2, %3)\n").arg(center.X(), 0, 'f', 6).arg(center.Y(), 0, 'f', 6).arg(center.Z(), 0, 'f', 6);
        
        if (i < selectedShapes.size() - 1) {
            info += "\n";
        }
    }
    
    // 显示信息对话框
    QMessageBox::information(this, "对象参数", info);
}

void View3D::onContextMenuColor()
{
    if (m_context.IsNull()) {
        return;
    }
    
    // 获取选中的对象
    QList<Handle(AIS_Shape)> selectedShapes;
    m_context->InitSelected();
    while (m_context->MoreSelected()) {
        Handle(AIS_InteractiveObject) obj = m_context->SelectedInteractive();
        if (!obj.IsNull() && obj != m_viewCube) {
            Handle(AIS_Shape) shape = Handle(AIS_Shape)::DownCast(obj);
            if (!shape.IsNull()) {
                selectedShapes.append(shape);
            }
        }
        m_context->NextSelected();
    }
    
    if (selectedShapes.isEmpty()) {
        return;
    }
    
    // 显示颜色选择对话框（使用父窗口确保菜单关闭）
    QColor color = QColorDialog::getColor(Qt::white, this, "选择颜色");
    if (color.isValid()) {
        // 转换为OpenCascade颜色
        Quantity_Color occColor(color.redF(), color.greenF(), color.blueF(), Quantity_TOC_RGB);
        
        // 设置所有选中对象的颜色
        for (const auto& shape : selectedShapes) {
            m_context->SetColor(shape, occColor, Standard_False);
        }
        
        m_context->UpdateCurrentViewer();
        update();
    }
    
    // 菜单已经通过exec()自动关闭，这里不需要额外处理
}

void View3D::onContextMenuTransparency()
{
    if (m_context.IsNull()) {
        return;
    }
    
    // 获取选中的对象
    QList<Handle(AIS_Shape)> selectedShapes;
    m_context->InitSelected();
    while (m_context->MoreSelected()) {
        Handle(AIS_InteractiveObject) obj = m_context->SelectedInteractive();
        if (!obj.IsNull() && obj != m_viewCube) {
            Handle(AIS_Shape) shape = Handle(AIS_Shape)::DownCast(obj);
            if (!shape.IsNull()) {
                selectedShapes.append(shape);
            }
        }
        m_context->NextSelected();
    }
    
    if (selectedShapes.isEmpty()) {
        return;
    }
    
    // 创建透明度设置对话框
    QDialog dialog(this);
    dialog.setWindowTitle("透明度设置");
    dialog.setModal(true);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
    
    // 标签显示当前值
    QLabel* label = new QLabel("透明度: 0.00 (0为不透明，1为完全透明)", &dialog);
    mainLayout->addWidget(label);
    
    // 滑块（0-100，对应0.0-1.0）
    QSlider* slider = new QSlider(Qt::Horizontal, &dialog);
    slider->setMinimum(0);
    slider->setMaximum(100);
    slider->setValue(0);
    slider->setTickPosition(QSlider::TicksBelow);
    slider->setTickInterval(10);
    mainLayout->addWidget(slider);
    
    // 数值输入框（可选，用于精确输入）
    QHBoxLayout* inputLayout = new QHBoxLayout();
    QLabel* inputLabel = new QLabel("数值:", &dialog);
    QDoubleSpinBox* spinBox = new QDoubleSpinBox(&dialog);
    spinBox->setMinimum(0.0);
    spinBox->setMaximum(1.0);
    spinBox->setSingleStep(0.01);
    spinBox->setDecimals(2);
    spinBox->setValue(0.0);
    inputLayout->addWidget(inputLabel);
    inputLayout->addWidget(spinBox);
    inputLayout->addStretch();
    mainLayout->addLayout(inputLayout);
    
    // 按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    QPushButton* okButton = new QPushButton("确定", &dialog);
    QPushButton* cancelButton = new QPushButton("取消", &dialog);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号：滑块和数值框同步
    // 使用指针捕获selectedShapes，确保lambda中能正确访问
    QList<Handle(AIS_Shape)>* shapesPtr = &selectedShapes;
    connect(slider, &QSlider::valueChanged, [label, spinBox, shapesPtr, this](int value) {
        double transparency = value / 100.0;
        label->setText(QString("透明度: %1 (0为不透明，1为完全透明)").arg(transparency, 0, 'f', 2));
        spinBox->blockSignals(true);
        spinBox->setValue(transparency);
        spinBox->blockSignals(false);
        
        // 实时预览透明度变化
        for (const auto& shape : *shapesPtr) {
            m_context->SetTransparency(shape, transparency, Standard_False);
        }
        m_context->UpdateCurrentViewer();
        update();
    });
    
    // 使用更兼容的方式连接spinBox信号
    // 连接spinBox信号，同步更新滑块和标签
    connect(spinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), 
            [label, slider, shapesPtr, this](double value) {
        int sliderValue = static_cast<int>(value * 100);
        label->setText(QString("透明度: %1 (0为不透明，1为完全透明)").arg(value, 0, 'f', 2));
        slider->blockSignals(true);
        slider->setValue(sliderValue);
        slider->blockSignals(false);
        
        // 实时预览透明度变化
        for (const auto& shape : *shapesPtr) {
            m_context->SetTransparency(shape, value, Standard_False);
        }
        m_context->UpdateCurrentViewer();
        update();
    });
    
    // 连接按钮
    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    
    // 如果用户取消，恢复原始透明度
    double originalTransparency = 0.0;
    if (!selectedShapes.isEmpty()) {
        // 尝试获取第一个对象的当前透明度（如果可能）
        // 注意：OpenCascade可能不直接提供获取透明度的方法，这里假设初始为0
        originalTransparency = 0.0;
    }
    
    // 显示对话框
    if (dialog.exec() == QDialog::Accepted) {
        // 用户点击确定，透明度已经在实时预览中设置好了
        // 这里不需要额外操作
    } else {
        // 用户取消，恢复原始透明度
        for (const auto& shape : selectedShapes) {
            m_context->SetTransparency(shape, originalTransparency, Standard_False);
        }
        m_context->UpdateCurrentViewer();
        update();
    }
}

void View3D::onContextMenuHide()
{
    if (m_context.IsNull()) {
        return;
    }
    
    // 获取选中的对象
    QList<Handle(AIS_InteractiveObject)> selectedObjects;
    m_context->InitSelected();
    while (m_context->MoreSelected()) {
        Handle(AIS_InteractiveObject) obj = m_context->SelectedInteractive();
        if (!obj.IsNull() && obj != m_viewCube) {
            selectedObjects.append(obj);
        }
        m_context->NextSelected();
    }
    
    // 隐藏选中的对象
    for (const auto& obj : selectedObjects) {
        m_context->Erase(obj, Standard_False);
    }
    
    // 清除选择
    m_context->ClearSelected(Standard_False);
    m_context->UpdateCurrentViewer();
    update();
}

void View3D::onContextMenuShowOnly()
{
    if (m_context.IsNull() || m_document == nullptr) {
        return;
    }
    
    // 获取选中的对象
    QList<Handle(AIS_InteractiveObject)> selectedObjects;
    m_context->InitSelected();
    while (m_context->MoreSelected()) {
        Handle(AIS_InteractiveObject) obj = m_context->SelectedInteractive();
        if (!obj.IsNull() && obj != m_viewCube) {
            selectedObjects.append(obj);
        }
        m_context->NextSelected();
    }
    
    if (selectedObjects.isEmpty()) {
        return;
    }
    
    // 隐藏所有对象
    for (int i = 0; i < m_document->getShapeCount(); ++i) {
        Handle(AIS_Shape) shape = m_document->getAISShape(i);
        if (!shape.IsNull()) {
            m_context->Erase(shape, Standard_False);
        }
    }
    
    // 显示选中的对象
    for (const auto& obj : selectedObjects) {
        m_context->Display(obj, Standard_False);
    }
    
    // 显示ViewCube（如果存在）
    if (!m_viewCube.IsNull()) {
        m_context->Display(m_viewCube, Standard_False);
    }
    
    m_context->UpdateCurrentViewer();
    update();
}

void View3D::onContextMenuShowAll()
{
    if (m_context.IsNull() || m_document == nullptr) {
        return;
    }
    
    // 显示所有对象
    for (int i = 0; i < m_document->getShapeCount(); ++i) {
        Handle(AIS_Shape) shape = m_document->getAISShape(i);
        if (!shape.IsNull()) {
            m_context->Display(shape, Standard_False);
        }
    }
    
    // 显示ViewCube（如果存在）
    if (!m_viewCube.IsNull()) {
        m_context->Display(m_viewCube, Standard_False);
    }
    
    m_context->UpdateCurrentViewer();
    update();
}

void View3D::onContextMenuHideAll()
{
    if (m_context.IsNull() || m_document == nullptr) {
        return;
    }
    
    // 隐藏所有对象（除了ViewCube）
    for (int i = 0; i < m_document->getShapeCount(); ++i) {
        Handle(AIS_Shape) shape = m_document->getAISShape(i);
        if (!shape.IsNull()) {
            m_context->Erase(shape, Standard_False);
        }
    }
    
    // 清除选择
    m_context->ClearSelected(Standard_False);
    m_context->UpdateCurrentViewer();
    update();
}

void View3D::onContextMenuClear()
{
    if (m_document == nullptr) {
        return;
    }
    
    // 确认对话框
    int ret = QMessageBox::question(
        this, "确认", "确定要删除所有对象吗？",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        m_document->clear();
        if (m_view && !m_view.IsNull()) {
            m_view->Redraw();
        }
    }
}
