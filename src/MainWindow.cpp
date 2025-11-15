#include "MainWindow.h"
#include "View3D.h"
#include "Document.h"
#include "SelectionManager.h"
#include "FileIO.h"
#include "Modeling.h"
#include "TransformManager.h"
#include "ParameterDialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileInfo>
#include <QKeySequence>
#include <QMenu>
#include <QToolBar>
#include <QTimer>
#include <QDebug>
#include <TopoDS_Shape.hxx>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_view3D(nullptr)
    , m_document(nullptr)
    , m_selectionManager(nullptr)
    , m_selectionFilterCombo(nullptr)
    , m_statusLabel(nullptr)
{
    // 创建核心对象
    m_document = new Document(this);
    m_selectionManager = new SelectionManager(this);
    m_view3D = new View3D(this);
    m_view3D->setDocument(m_document);
    m_selectionManager->setView3D(m_view3D);
    m_selectionManager->setContext(m_view3D->getContext());
    
    setupUI();
    setupMenus();
    setupToolbars();
    setupDockWidgets();
    connectSignals();
    
    setWindowTitle("MyCad - 3D CAD Software");
    resize(1200, 800);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    setCentralWidget(m_view3D);
    
    // ViewCube 已在 View3D::init() 中创建并显示
    // OpenCascade 的 AIS_ViewCube 会自动管理位置
    
    m_statusLabel = new QLabel("就绪", this);
    statusBar()->addWidget(m_statusLabel);
}

void MainWindow::setupMenus()
{
    // 文件菜单
    QMenu* fileMenu = menuBar()->addMenu("文件(&F)");
    
    QAction* newAction = fileMenu->addAction("新建(&N)");
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::onNewFile);
    
    QAction* openAction = fileMenu->addAction("打开(&O)");
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenFile);
    
    QAction* importAction = fileMenu->addAction("导入(&I)");
    connect(importAction, &QAction::triggered, this, &MainWindow::onImportFile);
    
    fileMenu->addSeparator();
    
    QAction* saveAction = fileMenu->addAction("保存(&S)");
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveFile);
    
    QAction* exportAction = fileMenu->addAction("导出(&E)");
    connect(exportAction, &QAction::triggered, this, &MainWindow::onExportFile);
    
    fileMenu->addSeparator();
    QAction* exitAction = fileMenu->addAction("退出(&X)");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
    
    // 视图菜单
    QMenu* viewMenu = menuBar()->addMenu("视图(&V)");
    
    QAction* viewTopAction = viewMenu->addAction("顶视图");
    connect(viewTopAction, &QAction::triggered, this, &MainWindow::onViewTop);
    
    QAction* viewFrontAction = viewMenu->addAction("前视图");
    connect(viewFrontAction, &QAction::triggered, this, &MainWindow::onViewFront);
    
    QAction* viewLeftAction = viewMenu->addAction("左视图");
    connect(viewLeftAction, &QAction::triggered, this, &MainWindow::onViewLeft);
    
    QAction* viewRightAction = viewMenu->addAction("右视图");
    connect(viewRightAction, &QAction::triggered, this, &MainWindow::onViewRight);
    
    QAction* viewBackAction = viewMenu->addAction("后视图");
    connect(viewBackAction, &QAction::triggered, this, &MainWindow::onViewBack);
    
    QAction* viewBottomAction = viewMenu->addAction("底视图");
    connect(viewBottomAction, &QAction::triggered, this, &MainWindow::onViewBottom);
    
    QAction* viewIsoAction = viewMenu->addAction("等轴测视图");
    connect(viewIsoAction, &QAction::triggered, this, &MainWindow::onViewIso);
    
    viewMenu->addSeparator();
    QAction* fitAllAction = viewMenu->addAction("适应窗口");
    connect(fitAllAction, &QAction::triggered, m_view3D, &View3D::fitAll);
    
    // 建模菜单
    QMenu* modelingMenu = menuBar()->addMenu("建模(&M)");
    
    QAction* boxAction = modelingMenu->addAction("方块");
    connect(boxAction, &QAction::triggered, this, &MainWindow::onCreateBox);
    
    QAction* cylinderAction = modelingMenu->addAction("圆柱");
    connect(cylinderAction, &QAction::triggered, this, &MainWindow::onCreateCylinder);
    
    QAction* sphereAction = modelingMenu->addAction("球");
    connect(sphereAction, &QAction::triggered, this, &MainWindow::onCreateSphere);
    
    QAction* coneAction = modelingMenu->addAction("圆锥");
    connect(coneAction, &QAction::triggered, this, &MainWindow::onCreateCone);
    
    modelingMenu->addSeparator();
    
    QAction* extrudeAction = modelingMenu->addAction("拉伸");
    connect(extrudeAction, &QAction::triggered, this, &MainWindow::onCreateExtrude);
    
    QAction* sweepAction = modelingMenu->addAction("扫略");
    connect(sweepAction, &QAction::triggered, this, &MainWindow::onCreateSweep);
    
    // 编辑菜单
    QMenu* editMenu = menuBar()->addMenu("编辑(&E)");
    
    QAction* unionAction = editMenu->addAction("并集");
    connect(unionAction, &QAction::triggered, this, &MainWindow::onBooleanUnion);
    
    QAction* cutAction = editMenu->addAction("差集");
    connect(cutAction, &QAction::triggered, this, &MainWindow::onBooleanCut);
    
    QAction* intersectAction = editMenu->addAction("交集");
    connect(intersectAction, &QAction::triggered, this, &MainWindow::onBooleanIntersect);
    
    editMenu->addSeparator();
    
    QAction* moveAction = editMenu->addAction("平移");
    connect(moveAction, &QAction::triggered, this, &MainWindow::onTransformMove);
    
    QAction* rotateAction = editMenu->addAction("旋转");
    connect(rotateAction, &QAction::triggered, this, &MainWindow::onTransformRotate);
    
    QAction* mirrorAction = editMenu->addAction("镜像");
    connect(mirrorAction, &QAction::triggered, this, &MainWindow::onTransformMirror);
    
    QAction* arrayAction = editMenu->addAction("阵列");
    connect(arrayAction, &QAction::triggered, this, &MainWindow::onTransformArray);
    
    editMenu->addSeparator();
    
    QAction* pickPointAction = editMenu->addAction("拾取点");
    connect(pickPointAction, &QAction::triggered, this, &MainWindow::onPickPoint);
}

void MainWindow::setupToolbars()
{
    // 视图工具栏
    QToolBar* viewToolbar = addToolBar("视图");
    viewToolbar->addAction("顶视图", this, &MainWindow::onViewTop);
    viewToolbar->addAction("前视图", this, &MainWindow::onViewFront);
    viewToolbar->addAction("左视图", this, &MainWindow::onViewLeft);
    viewToolbar->addAction("等轴测", this, &MainWindow::onViewIso);
    viewToolbar->addSeparator();
    viewToolbar->addAction("适应窗口", m_view3D, &View3D::fitAll);
    
    // 建模工具栏
    QToolBar* modelingToolbar = addToolBar("建模");
    modelingToolbar->addAction("方块", this, &MainWindow::onCreateBox);
    modelingToolbar->addAction("圆柱", this, &MainWindow::onCreateCylinder);
    modelingToolbar->addAction("球", this, &MainWindow::onCreateSphere);
    modelingToolbar->addAction("圆锥", this, &MainWindow::onCreateCone);
}

void MainWindow::setupDockWidgets()
{
    // 选择过滤面板
    QDockWidget* selectionDock = new QDockWidget("选择过滤", this);
    QWidget* selectionWidget = new QWidget();
    QVBoxLayout* selectionLayout = new QVBoxLayout(selectionWidget);
    
    m_selectionFilterCombo = new QComboBox();
    m_selectionFilterCombo->addItem("无过滤", static_cast<int>(SelectionFilterType::None));
    m_selectionFilterCombo->addItem("点", static_cast<int>(SelectionFilterType::Vertex));
    m_selectionFilterCombo->addItem("边", static_cast<int>(SelectionFilterType::Edge));
    m_selectionFilterCombo->addItem("线", static_cast<int>(SelectionFilterType::Wire));
    m_selectionFilterCombo->addItem("面", static_cast<int>(SelectionFilterType::Face));
    m_selectionFilterCombo->addItem("体", static_cast<int>(SelectionFilterType::Solid));
    
    selectionLayout->addWidget(new QLabel("选择类型:"));
    selectionLayout->addWidget(m_selectionFilterCombo);
    selectionLayout->addStretch();
    
    selectionWidget->setLayout(selectionLayout);
    selectionDock->setWidget(selectionWidget);
    addDockWidget(Qt::RightDockWidgetArea, selectionDock);
}

void MainWindow::connectSignals()
{
    connect(m_selectionFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onSelectionFilterChanged);
}

void MainWindow::onNewFile()
{
    m_document->clear();
    m_view3D->fitAll();
    m_statusLabel->setText("新建文件");
}

void MainWindow::onOpenFile()
{
    QString filename = QFileDialog::getOpenFileName(this, "打开文件", "", 
                                                    "MyCad文件 (*.mycad);;所有文件 (*.*)");
    if (!filename.isEmpty()) {
        if (m_document->loadFromFile(filename)) {
            m_statusLabel->setText(QString("已打开: %1").arg(filename));
        } else {
            QMessageBox::warning(this, "错误", "无法打开文件");
        }
    }
}

void MainWindow::onImportFile()
{
    QString filter = FileIO::getFileFilter(true);
    QString filename = QFileDialog::getOpenFileName(this, "导入文件", "", filter);
    
    if (!filename.isEmpty()) {
        TopoDS_Shape shape;
        if (FileIO::importFile(filename, shape)) {
            m_document->addShape(shape, QFileInfo(filename).baseName());
            m_view3D->fitAll();
            m_statusLabel->setText(QString("已导入: %1").arg(filename));
        } else {
            QMessageBox::warning(this, "错误", "无法导入文件");
        }
    }
}

void MainWindow::onExportFile()
{
    if (m_document->isEmpty()) {
        QMessageBox::information(this, "提示", "文档为空，无法导出");
        return;
    }
    
    QString filter = FileIO::getFileFilter(false);
    QString filename = QFileDialog::getSaveFileName(this, "导出文件", "", filter);
    
    if (!filename.isEmpty()) {
        // 导出第一个形状（可以扩展为导出所有或选中的）
        TopoDS_Shape shape = m_document->getShape(0);
        if (FileIO::exportFile(filename, shape)) {
            m_statusLabel->setText(QString("已导出: %1").arg(filename));
        } else {
            QMessageBox::warning(this, "错误", "无法导出文件");
        }
    }
}

void MainWindow::onSaveFile()
{
    QString filename = QFileDialog::getSaveFileName(this, "保存文件", "", 
                                                    "MyCad文件 (*.mycad);;所有文件 (*.*)");
    if (!filename.isEmpty()) {
        if (!filename.endsWith(".mycad", Qt::CaseInsensitive)) {
            filename += ".mycad";
        }
        if (m_document->saveToFile(filename)) {
            m_statusLabel->setText(QString("已保存: %1").arg(filename));
        } else {
            QMessageBox::warning(this, "错误", "无法保存文件");
        }
    }
}

void MainWindow::onViewTop()
{
    m_view3D->setViewTop();
}

void MainWindow::onViewFront()
{
    m_view3D->setViewFront();
}

void MainWindow::onViewLeft()
{
    m_view3D->setViewLeft();
}

void MainWindow::onViewRight()
{
    m_view3D->setViewRight();
}

void MainWindow::onViewBack()
{
    m_view3D->setViewBack();
}

void MainWindow::onViewBottom()
{
    m_view3D->setViewBottom();
}

void MainWindow::onViewIso()
{
    m_view3D->setViewIso();
}

void MainWindow::onCreateBox()
{
    qDebug() << "MainWindow::onCreateBox() - 开始";
    
    ParameterDialog dialog("创建方块", this);
    dialog.addParameter("长度 (X)", 10.0, 0.1, 1000.0, 2);
    dialog.addParameter("宽度 (Y)", 10.0, 0.1, 1000.0, 2);
    dialog.addParameter("高度 (Z)", 10.0, 0.1, 1000.0, 2);
    
    if (dialog.exec() != QDialog::Accepted) {
        qDebug() << "MainWindow::onCreateBox() - 用户取消";
        return;
    }
    
    double dx = dialog.getParameter(0);
    double dy = dialog.getParameter(1);
    double dz = dialog.getParameter(2);
    
    qDebug() << "MainWindow::onCreateBox() - 参数:" << dx << dy << dz;
    qDebug() << "MainWindow::onCreateBox() - 调用 Modeling::createBox()";
    TopoDS_Shape box = Modeling::createBox(dx, dy, dz);
    if (box.IsNull()) {
        qWarning() << "MainWindow::onCreateBox() - 创建方块失败！";
        QMessageBox::warning(this, "错误", "创建方块失败！");
        return;
    }
    qDebug() << "MainWindow::onCreateBox() - 方块创建成功";
    
    qDebug() << "MainWindow::onCreateBox() - 调用 addShape()";
    m_document->addShape(box, "Box");
    qDebug() << "MainWindow::onCreateBox() - addShape() 调用完成";
    
    // 延迟调用fitAll，确保视图已更新
    qDebug() << "MainWindow::onCreateBox() - 设置延迟调用 fitAll()";
    QTimer::singleShot(100, [this]() {
        qDebug() << "MainWindow::onCreateBox() - 延迟回调执行";
        if (m_view3D && !m_view3D->getView().IsNull()) {
            qDebug() << "MainWindow::onCreateBox() - 调用 fitAll()";
            m_view3D->fitAll();
            qDebug() << "MainWindow::onCreateBox() - fitAll() 完成";
        } else {
            qWarning() << "MainWindow::onCreateBox() - View3D或视图为空";
        }
    });
    qDebug() << "MainWindow::onCreateBox() - 完成";
}

void MainWindow::onCreateCylinder()
{
    ParameterDialog dialog("创建圆柱", this);
    dialog.addParameter("半径", 5.0, 0.1, 1000.0, 2);
    dialog.addParameter("高度", 10.0, 0.1, 1000.0, 2);
    
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    
    double radius = dialog.getParameter(0);
    double height = dialog.getParameter(1);
    
    TopoDS_Shape cylinder = Modeling::createCylinder(radius, height);
    m_document->addShape(cylinder, "Cylinder");
    m_view3D->fitAll();
}

void MainWindow::onCreateSphere()
{
    ParameterDialog dialog("创建球", this);
    dialog.addParameter("半径", 5.0, 0.1, 1000.0, 2);
    
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    
    double radius = dialog.getParameter(0);
    
    TopoDS_Shape sphere = Modeling::createSphere(radius);
    m_document->addShape(sphere, "Sphere");
    m_view3D->fitAll();
}

void MainWindow::onCreateCone()
{
    ParameterDialog dialog("创建圆锥", this);
    dialog.addParameter("底部半径", 5.0, 0.1, 1000.0, 2);
    dialog.addParameter("顶部半径", 2.0, 0.0, 1000.0, 2);
    dialog.addParameter("高度", 10.0, 0.1, 1000.0, 2);
    
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    
    double radius1 = dialog.getParameter(0);
    double radius2 = dialog.getParameter(1);
    double height = dialog.getParameter(2);
    
    TopoDS_Shape cone = Modeling::createCone(radius1, radius2, height);
    m_document->addShape(cone, "Cone");
    m_view3D->fitAll();
}

void MainWindow::onCreateExtrude()
{
    QMessageBox::information(this, "提示", "请先选择一个面或线框，然后使用此功能");
    // TODO: 实现拉伸功能
}

void MainWindow::onCreateSweep()
{
    QMessageBox::information(this, "提示", "请先选择轮廓和路径，然后使用此功能");
    // TODO: 实现扫略功能
}

void MainWindow::onSelectionFilterChanged(int index)
{
    QVariant data = m_selectionFilterCombo->itemData(index);
    if (!data.isValid()) {
        qWarning() << "MainWindow::onSelectionFilterChanged - 无效的 itemData，index:" << index;
        return;
    }
    
    SelectionFilterType type = static_cast<SelectionFilterType>(data.toInt());
    qDebug() << "MainWindow::onSelectionFilterChanged - index:" << index << "type:" << static_cast<int>(type);
    m_selectionManager->setFilterType(type);
}

void MainWindow::onBooleanUnion()
{
    // 确保SelectionManager的上下文是最新的
    if (m_view3D && !m_view3D->getContext().IsNull()) {
        m_selectionManager->setContext(m_view3D->getContext());
    }
    
    auto shapes = m_selectionManager->getSelectedShapes();
    auto aisObjects = m_selectionManager->getSelectedObjects();
    
    qDebug() << "布尔并集运算 - 选中的形状数量:" << shapes.size();
    qDebug() << "布尔并集运算 - 选中的AIS对象数量:" << aisObjects.size();
    
    if (shapes.size() < 2) {
        QMessageBox::information(this, "提示", QString("请至少选择两个对象进行并集运算（当前选中：%1个）").arg(shapes.size()));
        return;
    }
    
    // 提取AIS_Shape对象
    QList<Handle(AIS_Shape)> aisShapes;
    for (const auto& obj : aisObjects) {
        Handle(AIS_Shape) aisShape = Handle(AIS_Shape)::DownCast(obj);
        if (!aisShape.IsNull()) {
            aisShapes.append(aisShape);
        }
    }
    
    if (aisShapes.size() != shapes.size()) {
        qWarning() << "警告：AIS对象数量与形状数量不匹配";
    }
    
    TransformManager manager(this);
    manager.setDocument(m_document);
    if (manager.booleanUnion(shapes, aisShapes)) {
        // 清除选择
        m_selectionManager->clearSelection();
        m_view3D->fitAll();
    } else {
        QMessageBox::warning(this, "错误", "布尔并集运算失败");
    }
}

void MainWindow::onBooleanCut()
{
    // 确保SelectionManager的上下文是最新的
    if (m_view3D && !m_view3D->getContext().IsNull()) {
        m_selectionManager->setContext(m_view3D->getContext());
    }
    
    auto shapes = m_selectionManager->getSelectedShapes();
    auto aisObjects = m_selectionManager->getSelectedObjects();
    
    qDebug() << "布尔差集运算 - 选中的形状数量:" << shapes.size();
    qDebug() << "布尔差集运算 - 选中的AIS对象数量:" << aisObjects.size();
    
    if (shapes.size() < 2) {
        QMessageBox::information(this, "提示", QString("请至少选择两个对象进行差集运算（当前选中：%1个）").arg(shapes.size()));
        return;
    }
    
    // 提取AIS_Shape对象
    QList<Handle(AIS_Shape)> aisShapes;
    for (const auto& obj : aisObjects) {
        Handle(AIS_Shape) aisShape = Handle(AIS_Shape)::DownCast(obj);
        if (!aisShape.IsNull()) {
            aisShapes.append(aisShape);
        }
    }
    
    TransformManager manager(this);
    manager.setDocument(m_document);
    if (manager.booleanCut(shapes, aisShapes)) {
        // 清除选择
        m_selectionManager->clearSelection();
        m_view3D->fitAll();
    } else {
        QMessageBox::warning(this, "错误", "布尔差集运算失败");
    }
}

void MainWindow::onBooleanIntersect()
{
    // 确保SelectionManager的上下文是最新的
    if (m_view3D && !m_view3D->getContext().IsNull()) {
        m_selectionManager->setContext(m_view3D->getContext());
    }
    
    auto shapes = m_selectionManager->getSelectedShapes();
    auto aisObjects = m_selectionManager->getSelectedObjects();
    
    qDebug() << "布尔交集运算 - 选中的形状数量:" << shapes.size();
    qDebug() << "布尔交集运算 - 选中的AIS对象数量:" << aisObjects.size();
    
    if (shapes.size() < 2) {
        QMessageBox::information(this, "提示", QString("请至少选择两个对象进行交集运算（当前选中：%1个）").arg(shapes.size()));
        return;
    }
    
    // 提取AIS_Shape对象
    QList<Handle(AIS_Shape)> aisShapes;
    for (const auto& obj : aisObjects) {
        Handle(AIS_Shape) aisShape = Handle(AIS_Shape)::DownCast(obj);
        if (!aisShape.IsNull()) {
            aisShapes.append(aisShape);
        }
    }
    
    TransformManager manager(this);
    manager.setDocument(m_document);
    if (manager.booleanIntersect(shapes, aisShapes)) {
        // 清除选择
        m_selectionManager->clearSelection();
        m_view3D->fitAll();
    } else {
        QMessageBox::warning(this, "错误", "布尔交集运算失败");
    }
}

void MainWindow::onTransformMove()
{
    QMessageBox::information(this, "提示", "平移功能开发中");
    // TODO: 实现平移对话框
}

void MainWindow::onTransformRotate()
{
    QMessageBox::information(this, "提示", "旋转功能开发中");
    // TODO: 实现旋转对话框
}

void MainWindow::onTransformMirror()
{
    QMessageBox::information(this, "提示", "镜像功能开发中");
    // TODO: 实现镜像对话框
}

void MainWindow::onTransformArray()
{
    QMessageBox::information(this, "提示", "阵列功能开发中");
    // TODO: 实现阵列对话框
}

void MainWindow::onPickPoint()
{
    m_statusLabel->setText("点击模型上的点进行拾取");
    // 拾取模式将在View3D的鼠标事件中处理
}

