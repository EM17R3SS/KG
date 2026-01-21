#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <vector>

struct Point3D {
    double x, y, z;
    Point3D(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
};

struct Line {
    Point3D start, end;
    bool visible;
    Line(Point3D s, Point3D e, bool v = true) : start(s), end(e), visible(v) {}
};

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    enum Theme {
        BEZIER_CURVE,
        BSPLINE_SURFACE,
        LINE_CLIPPING,
        ZBUFFER,
        RAY_TRACING
    };

    GLWidget(QWidget* parent = nullptr);

    void initializeControlPoints();
    void calculateBezierCurve();
    void updateControlPoint(int index, double x, double y, double z);
    std::vector<Point3D> getControlPoints() const;
    void setShowControlPolygon(bool show);
    void setCurrentTheme(Theme theme);

    void setBSplineOrder(int order);
    void setClippingWindow(double left, double right, double bottom, double top);
    void setZBufferObjectsCount(int count);
    void setRayTracingQuality(int quality);

public slots:
    void generateBSplineSurface();
    void performClipping();
    void generateLines();
    void generateZBufferScene();
    void renderRayTracing();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void drawCoordinateAxes();
    void drawControlPoints();
    void drawControlPolygon();
    void drawBezierCurve();
    void drawBSplineSurface();
    void drawLineClipping();
    void drawZBuffer();
    void drawRayTracing();
    Point3D calculateBezierPoint(int startIndex, double t);
    void applySmoothnessConditions();
    // Вспомогательные методы
    void drawSimpleSphere(double radius);
    void drawSimpleCube(double size);
    void drawPyramid(double size);
    void drawTorus(double R, double r);

    // Методы для отсечения отрезков
    bool isPointInsideWindow(double x, double y);
    int computeRegionCode(double x, double y);

    // Методы для трассировки лучей
    void generateRayTracingScene();
    void drawRayTracingRoom();
    void drawRayTracingObjects();
    void drawRays();
    void drawQualityInfo();

    // Данные для разных тем
    std::vector<Point3D> controlPoints;
    std::vector<Point3D> bezierCurve;

    // B-spline данные
    std::vector<std::vector<Point3D>> bsplineControlNet;
    std::vector<Point3D> bsplineSurface;

    // Line clipping данные
    std::vector<Line> originalLines;
    std::vector<Line> clippedLines;

    // Z-buffer данные
    std::vector<std::vector<Point3D>> zbufferObjects;

    // Данные для трассировки лучей
    std::vector<Point3D> rays;
    std::vector<Point3D> rayHits;

    float rotationX, rotationY;
    QPoint lastMousePos;
    bool isRotating;
    bool showControlPolygon;
    Theme currentTheme;

    // Параметры
    int bsplineOrder;
    int currentBSplineOrder;
    double clipLeft, clipRight, clipBottom, clipTop;
    int zbufferObjectsCount;
    int rayTracingQuality;
    int maxRays;
    bool showRays;
};

#endif // GLWIDGET_H
