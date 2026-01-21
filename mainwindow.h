#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>

class GLWidget;
class PointTableWidget;
class QCheckBox;
class QLabel;
class QComboBox;
class QGroupBox;
class QSpinBox;
class QDoubleSpinBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);

private slots:
    void onPointChanged(int row, int column);
    void onResetPoints();
    void onAboutClicked();
    void updateStatus();
    void onThemeChanged(int index);
    void onBSplineOrderChanged(int order);
    void onClippingWindowChanged();
    void onZBufferObjectsChanged(int count);
    void onRayTracingQualityChanged(int quality);

private:
    void createControlPanels();
    QWidget* createBezierControls();
    QWidget* createBSplineControls();
    QWidget* createClippingControls();
    QWidget* createZBufferControls();
    QWidget* createRayTracingControls();
    void updatePointTable();
    void setupBezierCurveTheme();
    void setupBSplineSurfaceTheme();
    void setupLineClippingTheme();
    void setupZBufferTheme();
    void setupRayTracingTheme();

    QStackedWidget* stackedWidget;
    QStackedWidget* controlStackedWidget;
    GLWidget* glWidget;
    PointTableWidget* pointTable;
    QCheckBox* showPolygonCheckBox;
    QLabel* statusLabel;
    QComboBox* themeComboBox;

    QSpinBox* bsplineOrderSpinBox;
    QDoubleSpinBox* clipLeftSpinBox;
    QDoubleSpinBox* clipRightSpinBox;
    QDoubleSpinBox* clipBottomSpinBox;
    QDoubleSpinBox* clipTopSpinBox;
    QSpinBox* zbufferObjectsSpinBox;
    QSpinBox* rayTracingQualitySpinBox;
};

#endif // MAINWINDOW_H
