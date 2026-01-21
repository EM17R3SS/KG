#include "mainwindow.h"
#include "glwidget.h"
#include "pointtablewidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>
#include <QCheckBox>
#include <QComboBox>
#include <QStackedWidget>
#include <QHeaderView>
#include <QSpinBox>
#include <QDoubleSpinBox>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("Компьютерная графика - Все темы");
    setMinimumSize(1200, 800);

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    stackedWidget = new QStackedWidget(this);
    glWidget = new GLWidget(this);
    stackedWidget->addWidget(glWidget);

    controlStackedWidget = new QStackedWidget(this);

    themeComboBox = new QComboBox();
    themeComboBox->addItem("1. Кривая Безье 3-й степени");
    themeComboBox->addItem("2. B-сплайновые поверхности");
    themeComboBox->addItem("3. Алгоритм отсечения отрезков");
    themeComboBox->addItem("4. Z-буфер для многогранников");
    themeComboBox->addItem("5. Трассировка лучей");

    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->addWidget(stackedWidget, 1);

    QWidget* rightPanel = new QWidget;
    rightPanel->setMaximumWidth(400);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);

    QGroupBox* themeGroup = new QGroupBox("Выбор темы");
    QVBoxLayout* themeLayout = new QVBoxLayout;
    themeLayout->addWidget(themeComboBox);
    themeGroup->setLayout(themeLayout);

    rightLayout->addWidget(themeGroup);
    rightLayout->addWidget(controlStackedWidget);
    rightLayout->addStretch(1);

    mainLayout->addWidget(rightPanel);

    createControlPanels();

    connect(themeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onThemeChanged);

    setupBezierCurveTheme();
    updatePointTable();
}

void MainWindow::createControlPanels()
{
    controlStackedWidget->addWidget(createBezierControls());
    controlStackedWidget->addWidget(createBSplineControls());
    controlStackedWidget->addWidget(createClippingControls());
    controlStackedWidget->addWidget(createZBufferControls());
    controlStackedWidget->addWidget(createRayTracingControls());
}

QWidget* MainWindow::createBezierControls()
{
    QWidget* panel = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(panel);

    QGroupBox* pointsGroup = new QGroupBox("Управляющие точки кривой Безье");
    QVBoxLayout* pointsLayout = new QVBoxLayout;

    QLabel* pointsLabel = new QLabel("Измените значения в таблице для редактирования точек:");
    pointsLabel->setWordWrap(true);
    pointsLayout->addWidget(pointsLabel);

    pointTable = new PointTableWidget(this);
    pointsLayout->addWidget(pointTable);

    QPushButton* resetPointsButton = new QPushButton("Сбросить точки");
    pointsLayout->addWidget(resetPointsButton);

    pointsGroup->setLayout(pointsLayout);

    QGroupBox* displayGroup = new QGroupBox("Отображение");
    QVBoxLayout* displayLayout = new QVBoxLayout;

    showPolygonCheckBox = new QCheckBox("Показывать контрольный полигон");
    showPolygonCheckBox->setChecked(true);
    displayLayout->addWidget(showPolygonCheckBox);

    displayGroup->setLayout(displayLayout);

    statusLabel = new QLabel;
    statusLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    statusLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(pointsGroup);
    layout->addWidget(displayGroup);
    layout->addWidget(statusLabel);

    connect(resetPointsButton, &QPushButton::clicked, this, &MainWindow::onResetPoints);
    connect(showPolygonCheckBox, &QCheckBox::toggled, glWidget, &GLWidget::setShowControlPolygon);
    connect(pointTable, &QTableWidget::cellChanged, this, &MainWindow::onPointChanged);

    return panel;
}

QWidget* MainWindow::createBSplineControls()
{
    QWidget* panel = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(panel);

    QGroupBox* bsplineGroup = new QGroupBox("Параметры B-сплайновой поверхности");
    QVBoxLayout* bsplineLayout = new QVBoxLayout;

    QLabel* orderLabel = new QLabel("Порядок B-сплайна:");
    bsplineOrderSpinBox = new QSpinBox;
    bsplineOrderSpinBox->setRange(2, 5);
    bsplineOrderSpinBox->setValue(3);

    bsplineLayout->addWidget(orderLabel);
    bsplineLayout->addWidget(bsplineOrderSpinBox);

    QPushButton* generateButton = new QPushButton("Сгенерировать поверхность");
    bsplineLayout->addWidget(generateButton);

    bsplineGroup->setLayout(bsplineLayout);

    QGroupBox* infoGroup = new QGroupBox("Информация");
    QVBoxLayout* infoLayout = new QVBoxLayout;
    QLabel* infoLabel = new QLabel("B-сплайновая поверхность на основе задающего многогранника");
    infoLabel->setWordWrap(true);
    infoLayout->addWidget(infoLabel);
    infoGroup->setLayout(infoLayout);

    layout->addWidget(bsplineGroup);
    layout->addWidget(infoGroup);

    connect(bsplineOrderSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onBSplineOrderChanged);
    connect(generateButton, &QPushButton::clicked, glWidget, &GLWidget::generateBSplineSurface);

    return panel;
}

QWidget* MainWindow::createClippingControls()
{
    QWidget* panel = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(panel);

    QGroupBox* windowGroup = new QGroupBox("Параметры окна отсечения");
    QGridLayout* windowLayout = new QGridLayout;

    windowLayout->addWidget(new QLabel("Левая граница:"), 0, 0);
    clipLeftSpinBox = new QDoubleSpinBox;
    clipLeftSpinBox->setRange(-10, 10);
    clipLeftSpinBox->setValue(-3);
    windowLayout->addWidget(clipLeftSpinBox, 0, 1);

    windowLayout->addWidget(new QLabel("Правая граница:"), 1, 0);
    clipRightSpinBox = new QDoubleSpinBox;
    clipRightSpinBox->setRange(-10, 10);
    clipRightSpinBox->setValue(3);
    windowLayout->addWidget(clipRightSpinBox, 1, 1);

    windowLayout->addWidget(new QLabel("Нижняя граница:"), 2, 0);
    clipBottomSpinBox = new QDoubleSpinBox;
    clipBottomSpinBox->setRange(-10, 10);
    clipBottomSpinBox->setValue(-3);
    windowLayout->addWidget(clipBottomSpinBox, 2, 1);

    windowLayout->addWidget(new QLabel("Верхняя граница:"), 3, 0);
    clipTopSpinBox = new QDoubleSpinBox;
    clipTopSpinBox->setRange(-10, 10);
    clipTopSpinBox->setValue(3);
    windowLayout->addWidget(clipTopSpinBox, 3, 1);

    windowGroup->setLayout(windowLayout);

    QPushButton* clipButton = new QPushButton("Выполнить отсечение");
    QPushButton* generateLinesButton = new QPushButton("Сгенерировать отрезки");

    QGroupBox* infoGroup = new QGroupBox("Информация");
    QVBoxLayout* infoLayout = new QVBoxLayout;
    QLabel* infoLabel = new QLabel("Алгоритм отсечения отрезков прямоугольным окном с использованием 4-битного кода");
    infoLabel->setWordWrap(true);
    infoLayout->addWidget(infoLabel);
    infoGroup->setLayout(infoLayout);

    layout->addWidget(windowGroup);
    layout->addWidget(generateLinesButton);
    layout->addWidget(clipButton);
    layout->addWidget(infoGroup);

    connect(clipLeftSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onClippingWindowChanged);
    connect(clipRightSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onClippingWindowChanged);
    connect(clipBottomSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onClippingWindowChanged);
    connect(clipTopSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onClippingWindowChanged);
    connect(clipButton, &QPushButton::clicked, glWidget, &GLWidget::performClipping);
    connect(generateLinesButton, &QPushButton::clicked, glWidget, &GLWidget::generateLines);

    return panel;
}

QWidget* MainWindow::createZBufferControls()
{
    QWidget* panel = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(panel);

    QGroupBox* objectsGroup = new QGroupBox("Параметры сцены");
    QVBoxLayout* objectsLayout = new QVBoxLayout;

    QLabel* objectsLabel = new QLabel("Количество многогранников:");
    zbufferObjectsSpinBox = new QSpinBox;
    zbufferObjectsSpinBox->setRange(1, 10);
    zbufferObjectsSpinBox->setValue(3);

    objectsLayout->addWidget(objectsLabel);
    objectsLayout->addWidget(zbufferObjectsSpinBox);

    QPushButton* generateButton = new QPushButton("Сгенерировать сцену");
    objectsLayout->addWidget(generateButton);

    objectsGroup->setLayout(objectsLayout);

    QGroupBox* infoGroup = new QGroupBox("Информация");
    QVBoxLayout* infoLayout = new QVBoxLayout;
    QLabel* infoLabel = new QLabel("Алгоритм Z-буфера для определения видимости многогранников");
    infoLabel->setWordWrap(true);
    infoLayout->addWidget(infoLabel);
    infoGroup->setLayout(infoLayout);

    layout->addWidget(objectsGroup);
    layout->addWidget(infoGroup);

    connect(zbufferObjectsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onZBufferObjectsChanged);
    connect(generateButton, &QPushButton::clicked, glWidget, &GLWidget::generateZBufferScene);

    return panel;
}

QWidget* MainWindow::createRayTracingControls() // ДОБАВЬТЕ QWidget*
{
    QWidget* panel = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(panel);

    QGroupBox* qualityGroup = new QGroupBox("Качество трассировки лучей");
    QVBoxLayout* qualityLayout = new QVBoxLayout;

    QLabel* qualityLabel = new QLabel("Уровень качества:");
    rayTracingQualitySpinBox = new QSpinBox;
    rayTracingQualitySpinBox->setRange(1, 5);
    rayTracingQualitySpinBox->setValue(3);
    rayTracingQualitySpinBox->setPrefix("Уровень ");

    QLabel* description = new QLabel(
        "1: Базовые лучи\n"
        "2: Больше лучей\n"
        "3: Точки попадания\n"
        "4: Отражения\n"
        "5: Максимальная детализация"
        );
    description->setWordWrap(true);

    QPushButton* renderButton = new QPushButton("Обновить трассировку");

    qualityLayout->addWidget(qualityLabel);
    qualityLayout->addWidget(rayTracingQualitySpinBox);
    qualityLayout->addWidget(description);
    qualityLayout->addWidget(renderButton);

    qualityGroup->setLayout(qualityLayout);

    QGroupBox* infoGroup = new QGroupBox("Информация");
    QVBoxLayout* infoLayout = new QVBoxLayout;
    QLabel* infoLabel = new QLabel(
        "Трассировка лучей моделирует путь света через сцену.\n"
        "При повышении качества добавляются:\n"
        "- Больше лучей\n"
        "- Точки попадания\n"
        "- Отражения\n"
        "- Тени и эффекты"
        );
    infoLabel->setWordWrap(true);
    infoLayout->addWidget(infoLabel);
    infoGroup->setLayout(infoLayout);

    layout->addWidget(qualityGroup);
    layout->addWidget(infoGroup);

    connect(rayTracingQualitySpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onRayTracingQualityChanged);
    connect(renderButton, &QPushButton::clicked, glWidget, &GLWidget::renderRayTracing);

    return panel; // НЕ ЗАБУДЬТЕ ВЕРНУТЬ panel
}

void MainWindow::setupBezierCurveTheme()
{
    controlStackedWidget->setCurrentIndex(0);
    glWidget->setCurrentTheme(GLWidget::BEZIER_CURVE);
    updatePointTable();
    updateStatus();
}

void MainWindow::setupBSplineSurfaceTheme()
{
    controlStackedWidget->setCurrentIndex(1);
    glWidget->setCurrentTheme(GLWidget::BSPLINE_SURFACE);
    updateStatus();
}

void MainWindow::setupLineClippingTheme()
{
    controlStackedWidget->setCurrentIndex(2);
    glWidget->setCurrentTheme(GLWidget::LINE_CLIPPING);
    updateStatus();
}

void MainWindow::setupZBufferTheme()
{
    controlStackedWidget->setCurrentIndex(3);
    glWidget->setCurrentTheme(GLWidget::ZBUFFER);
    updateStatus();
}

void MainWindow::setupRayTracingTheme()
{
    controlStackedWidget->setCurrentIndex(4);
    glWidget->setCurrentTheme(GLWidget::RAY_TRACING);
    updateStatus();
}

void MainWindow::onThemeChanged(int index)
{
    switch(index) {
    case 0: setupBezierCurveTheme(); break;
    case 1: setupBSplineSurfaceTheme(); break;
    case 2: setupLineClippingTheme(); break;
    case 3: setupZBufferTheme(); break;
    case 4: setupRayTracingTheme(); break;
    }
}

void MainWindow::onBSplineOrderChanged(int order) {
    glWidget->setBSplineOrder(order);
}

void MainWindow::onClippingWindowChanged() {
    glWidget->setClippingWindow(
        clipLeftSpinBox->value(),
        clipRightSpinBox->value(),
        clipBottomSpinBox->value(),
        clipTopSpinBox->value()
        );
}

void MainWindow::onZBufferObjectsChanged(int count) {
    glWidget->setZBufferObjectsCount(count);
}

void MainWindow::onRayTracingQualityChanged(int quality) {
    glWidget->setRayTracingQuality(quality);
}

void MainWindow::onPointChanged(int row, int column)
{
    std::vector<Point3D> points = pointTable->getPoints();
    if (row < points.size()) {
        glWidget->updateControlPoint(row, points[row].x, points[row].y, points[row].z);
    }
    updateStatus();
}

void MainWindow::onResetPoints()
{
    glWidget->initializeControlPoints();
    glWidget->calculateBezierCurve();
    updatePointTable();
    updateStatus();
}

void MainWindow::updatePointTable()
{
    pointTable->blockSignals(true);
    pointTable->updatePoints(glWidget->getControlPoints());
    pointTable->blockSignals(false);
}

void MainWindow::onAboutClicked()
{
    QMessageBox::about(this, "О программе",
                       "<h3>Компьютерная графика - Все темы</h3>"
                       "<p>Программа включает реализацию 5 тем:</p>"
                       "<p>1. Кривая Безье 3-й степени</p>"
                       "<p>2. B-сплайновые поверхности</p>"
                       "<p>3. Алгоритм отсечения отрезков</p>"
                       "<p>4. Z-буфер для многогранников</p>"
                       "<p>5. Трассировка лучей</p>");
}

void MainWindow::updateStatus()
{
    auto points = glWidget->getControlPoints();
    QString themeName = themeComboBox->currentText();
    QString status = QString("Тема: %1\nТочек: %2")
                         .arg(themeName)
                         .arg(points.size());
    statusLabel->setText(status);
}
