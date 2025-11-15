#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QGroupBox>

class View3D;
class Document;
class SelectionManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 文件操作
    void onImportFile();
    void onExportFile();
    void onSaveFile();
    void onOpenFile();
    void onNewFile();
    
    // 视图操作
    void onViewTop();
    void onViewFront();
    void onViewLeft();
    void onViewRight();
    void onViewBack();
    void onViewBottom();
    void onViewIso();
    
    // 体素建模
    void onCreateBox();
    void onCreateCylinder();
    void onCreateSphere();
    void onCreateCone();
    void onCreateExtrude();
    void onCreateSweep();
    
    // 选择过滤
    void onSelectionFilterChanged(int index);
    
    // 变换操作
    void onBooleanUnion();
    void onBooleanCut();
    void onBooleanIntersect();
    void onTransformMove();
    void onTransformRotate();
    void onTransformMirror();
    void onTransformArray();
    
    // 鼠标拾取
    void onPickPoint();

private:
    void setupUI();
    void setupMenus();
    void setupToolbars();
    void setupDockWidgets();
    void connectSignals();
    
    View3D* m_view3D;
    Document* m_document;
    SelectionManager* m_selectionManager;
    
    // UI组件
    QComboBox* m_selectionFilterCombo;
    QLabel* m_statusLabel;
};

#endif // MAINWINDOW_H

