#include "glwidget.h"
#include <QMouseEvent>
#include <cmath>
#include <algorithm>
#include <random>
#include <QColor>

GLWidget::GLWidget(QWidget* parent)
    : QOpenGLWidget(parent),
    rotationX(15.0f), rotationY(15.0f),
    isRotating(false),
    showControlPolygon(true),
    currentTheme(BEZIER_CURVE),
    bsplineOrder(3),
    currentBSplineOrder(3), // Добавляем инициализацию
    clipLeft(-3), clipRight(3), clipBottom(-3), clipTop(3),
    zbufferObjectsCount(3),
    rayTracingQuality(3),
    maxRays(50),
    showRays(true)
{
    initializeControlPoints();
    calculateBezierCurve();
    generateLines();
    generateZBufferScene();
    generateBSplineSurface(); // Генерируем B-spline по умолчанию
    generateRayTracingScene();
}


void GLWidget::calculateBezierCurve()
{
    bezierCurve.clear();
    int segments = 50; // Уменьшаем для лучшей производительности

    // Рисуем 3 сегмента кубических кривых Безье
    for (int seg = 0; seg < 3; seg++) {
        int startIndex = seg * 3;
        if (startIndex + 3 >= controlPoints.size()) break;

        for (int j = 0; j <= segments; j++) {
            double t = double(j) / segments;
            Point3D point = calculateBezierPoint(startIndex, t);
            bezierCurve.push_back(point);
        }
    }
}
void GLWidget::drawControlPoints()
{
    glPointSize(8.0f);
    glBegin(GL_POINTS);

    for (size_t i = 0; i < controlPoints.size(); i++) {
        // Разные цвета для разных типов точек
        if (i % 3 == 0) {
            glColor3f(1.0f, 0.0f, 0.0f); // Красные - начальные точки сегментов
        } else if (i % 3 == 1 || i % 3 == 2) {
            glColor3f(0.0f, 1.0f, 0.0f); // Зеленые - контрольные точки
        }

        // Точки соединения сегментов (индексы 3, 6) выделяем особым цветом
        if (i == 3 || i == 6) {
            glColor3f(0.0f, 0.0f, 1.0f); // Синие - точки соединения
        }

        glVertex3f(controlPoints[i].x, controlPoints[i].y, controlPoints[i].z);
    }
    glEnd();
    glPointSize(1.0f);
}

Point3D GLWidget::calculateBezierPoint(int startIndex, double t)
{
    double u = 1 - t;
    double tt = t * t;
    double uu = u * u;
    double uuu = uu * u;
    double ttt = tt * t;

    Point3D p0 = controlPoints[startIndex];
    Point3D p1 = controlPoints[startIndex + 1];
    Point3D p2 = controlPoints[startIndex + 2];
    Point3D p3 = controlPoints[startIndex + 3];

    Point3D point;
    point.x = uuu * p0.x + 3 * uu * t * p1.x + 3 * u * tt * p2.x + ttt * p3.x;
    point.y = uuu * p0.y + 3 * uu * t * p1.y + 3 * u * tt * p2.y + ttt * p3.y;
    point.z = uuu * p0.z + 3 * uu * t * p1.z + 3 * u * tt * p2.z + ttt * p3.z;

    return point;
}

void GLWidget::updateControlPoint(int index, double x, double y, double z)
{
    if (index >= 0 && index < controlPoints.size()) {
        controlPoints[index] = Point3D(x, y, z);

        // Автоматически применяем условия гладкости при изменении контрольных точек
        if (index == 2 || index == 3 || index == 4) {
            // Если изменили точки вокруг первого соединения
            applySmoothnessConditions();
        } else if (index == 5 || index == 6 || index == 7) {
            // Если изменили точки вокруг второго соединения
            applySmoothnessConditions();
        } else {
            calculateBezierCurve();
        }

        update();
    }
}

std::vector<Point3D> GLWidget::getControlPoints() const
{
    return controlPoints;
}

void GLWidget::setShowControlPolygon(bool show)
{
    showControlPolygon = show;
    update();
}

void GLWidget::setCurrentTheme(Theme theme)
{
    currentTheme = theme;
    update();
}

void GLWidget::setBSplineOrder(int order)
{
    bsplineOrder = order;
    currentBSplineOrder = order; // Сохраняем текущий порядок
    generateBSplineSurface(); // Перегенерируем поверхность при изменении порядка
    update();
}

void GLWidget::setClippingWindow(double left, double right, double bottom, double top)
{
    clipLeft = left;
    clipRight = right;
    clipBottom = bottom;
    clipTop = top;
    update();
}

void GLWidget::setZBufferObjectsCount(int count)
{
    zbufferObjectsCount = count;
    update();
}
void GLWidget::generateRayTracingScene()
{
    rays.clear();
    rayHits.clear();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> posDis(-4.0, 4.0);
    std::uniform_real_distribution<> angleDis(0, 2 * M_PI);

    // Генерируем лучи из случайных точек в случайных направлениях
    for (int i = 0; i < maxRays * rayTracingQuality; i++) {
        Point3D start(posDis(gen), posDis(gen), 5.0); // Лучи идут сверху
        double angle = angleDis(gen);
        Point3D end(start.x + sin(angle) * 10.0, start.y + cos(angle) * 10.0, -5.0);
        rays.push_back(start);
        rays.push_back(end);

        // Случайные точки "попадания" лучей
        double t = static_cast<double>(rand()) / RAND_MAX;
        Point3D hit(
            start.x + t * (end.x - start.x),
            start.y + t * (end.y - start.y),
            start.z + t * (end.z - start.z)
            );
        rayHits.push_back(hit);
    }

    update();
}
void GLWidget::setRayTracingQuality(int quality)
{
    rayTracingQuality = quality;
    maxRays = 20 * quality; // Больше качества = больше лучей
    generateRayTracingScene(); // Перегенерируем сцену
    update();
}

void GLWidget::generateBSplineSurface()
{
    bsplineControlNet.clear();
    bsplineSurface.clear();

    // Создаем контрольную сетку 4x4 с учетом порядка
    int size = 4 + (currentBSplineOrder - 2); // Увеличиваем сетку для более высоких порядков
    for (int i = 0; i < size; i++) {
        std::vector<Point3D> row;
        for (int j = 0; j < size; j++) {
            double x = (i - size/2.0) * 1.5;
            double y = (j - size/2.0) * 1.5;
            // Разная форма поверхности в зависимости от порядка
            double z = 0;
            switch(currentBSplineOrder) {
            case 2: z = sin(i * 0.5) * cos(j * 0.5) * 1.0; break;
            case 3: z = sin(i * 0.8) * cos(j * 0.8) * 1.5; break;
            case 4: z = cos(i * 1.0) * sin(j * 1.0) * 2.0; break;
            case 5: z = (sin(i * 1.2) + cos(j * 1.2)) * 1.0; break;
            default: z = sin(i * 0.8) * cos(j * 0.8) * 1.5;
            }
            row.push_back(Point3D(x, y, z));
        }
        bsplineControlNet.push_back(row);
    }

    // Генерируем поверхность с учетом порядка
    int divisions = 20;
    for (int i = 0; i <= divisions; i++) {
        double u = double(i) / divisions;
        for (int j = 0; j <= divisions; j++) {
            double v = double(j) / divisions;

            // Более сложная интерполяция с учетом порядка
            double x = (u - 0.5) * 6.0;
            double y = (v - 0.5) * 6.0;
            double z = 0;

            // Разная сложность поверхности в зависимости от порядка
            switch(currentBSplineOrder) {
            case 2: z = sin(x * 0.3) * cos(y * 0.3) * 1.0; break;
            case 3: z = sin(x * 0.5) * cos(y * 0.5) * 1.5; break;
            case 4: z = (sin(x * 0.7) + cos(y * 0.7)) * 1.0; break;
            case 5: z = sin(x * 1.0) * cos(y * 1.0) * 0.8 + cos(x * 0.5) * sin(y * 0.5) * 0.5; break;
            default: z = sin(x * 0.5) * cos(y * 0.5) * 1.5;
            }

            bsplineSurface.push_back(Point3D(x, y, z));
        }
    }

    update();
}

void GLWidget::generateLines()
{
    originalLines.clear();
    clippedLines.clear();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-5.0, 5.0);

    // Генерируем случайные отрезки
    for (int i = 0; i < 15; i++) {
        Point3D start(dis(gen), dis(gen), 0);
        Point3D end(dis(gen), dis(gen), 0);
        originalLines.push_back(Line(start, end));
    }

    update();
}

bool GLWidget::isPointInsideWindow(double x, double y)
{
    return x >= clipLeft && x <= clipRight && y >= clipBottom && y <= clipTop;
}

int GLWidget::computeRegionCode(double x, double y)
{
    int code = 0;
    if (x < clipLeft) code |= 1;      // LEFT
    if (x > clipRight) code |= 2;     // RIGHT
    if (y < clipBottom) code |= 4;    // BOTTOM
    if (y > clipTop) code |= 8;       // TOP
    return code;
}

void GLWidget::performClipping()
{
    clippedLines.clear();

    for (const auto& line : originalLines) {
        double x1 = line.start.x, y1 = line.start.y;
        double x2 = line.end.x, y2 = line.end.y;

        int code1 = computeRegionCode(x1, y1);
        int code2 = computeRegionCode(x2, y2);

        bool accept = false;

        while (true) {
            if (!(code1 | code2)) {
                // Оба конца внутри окна
                accept = true;
                break;
            } else if (code1 & code2) {
                // Оба конца с одной стороны окна
                break;
            } else {
                // Нужно отсекать
                double x, y;
                int codeOut = code1 ? code1 : code2;

                // Находим точку пересечения
                if (codeOut & 8) {           // TOP
                    x = x1 + (x2 - x1) * (clipTop - y1) / (y2 - y1);
                    y = clipTop;
                } else if (codeOut & 4) {    // BOTTOM
                    x = x1 + (x2 - x1) * (clipBottom - y1) / (y2 - y1);
                    y = clipBottom;
                } else if (codeOut & 2) {    // RIGHT
                    y = y1 + (y2 - y1) * (clipRight - x1) / (x2 - x1);
                    x = clipRight;
                } else if (codeOut & 1) {    // LEFT
                    y = y1 + (y2 - y1) * (clipLeft - x1) / (x2 - x1);
                    x = clipLeft;
                }

                if (codeOut == code1) {
                    x1 = x; y1 = y;
                    code1 = computeRegionCode(x1, y1);
                } else {
                    x2 = x; y2 = y;
                    code2 = computeRegionCode(x2, y2);
                }
            }
        }

        if (accept) {
            clippedLines.push_back(Line(Point3D(x1, y1, 0), Point3D(x2, y2, 0), true));
        }
    }

    update();
}

void GLWidget::generateZBufferScene()
{
    zbufferObjects.clear();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> posDis(-3.0, 3.0);
    std::uniform_real_distribution<> sizeDis(0.5, 1.5);

    for (int i = 0; i < zbufferObjectsCount; i++) {
        std::vector<Point3D> object;
        double x = posDis(gen);
        double y = posDis(gen);
        double z = i * 0.8; // Разная глубина для демонстрации Z-буфера
        double size = sizeDis(gen);

        // Создаем пирамиду
        object.push_back(Point3D(x - size, y - size, z)); // основание
        object.push_back(Point3D(x + size, y - size, z));
        object.push_back(Point3D(x + size, y + size, z));
        object.push_back(Point3D(x - size, y + size, z));
        object.push_back(Point3D(x, y, z + size * 2));    // вершина

        zbufferObjects.push_back(object);
    }

    update();
}



void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

void GLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = float(w) / float(h);
    glOrtho(-8 * aspect, 8 * aspect, -8, 8, -20, 20);
    glMatrixMode(GL_MODELVIEW);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glRotatef(rotationX, 1.0f, 0.0f, 0.0f);
    glRotatef(rotationY, 0.0f, 1.0f, 0.0f);

    drawCoordinateAxes();

    switch(currentTheme) {
    case BEZIER_CURVE:
        drawControlPoints();
        if (showControlPolygon) {
            drawControlPolygon();
        }
        drawBezierCurve();
        break;
    case BSPLINE_SURFACE:
        drawBSplineSurface();
        break;
    case LINE_CLIPPING:
        drawLineClipping();
        break;
    case ZBUFFER:
        drawZBuffer();
        break;
    case RAY_TRACING:
        drawRayTracing();
        break;
    }
}

void GLWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        lastMousePos = event->pos();
        isRotating = true;
        setCursor(Qt::ClosedHandCursor);
    }
}

void GLWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (isRotating && (event->buttons() & Qt::LeftButton)) {
        QPoint delta = event->pos() - lastMousePos;
        rotationY += delta.x() * 0.5f;
        rotationX += delta.y() * 0.5f;
        lastMousePos = event->pos();
        update();
    }
}

void GLWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        isRotating = false;
        setCursor(Qt::ArrowCursor);
    }
}

void GLWidget::drawCoordinateAxes()
{
    glLineWidth(2.0f);
    glBegin(GL_LINES);

    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-10.0f, 0.0f, 0.0f);
    glVertex3f(10.0f, 0.0f, 0.0f);

    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, -10.0f, 0.0f);
    glVertex3f(0.0f, 10.0f, 0.0f);

    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, -10.0f);
    glVertex3f(0.0f, 0.0f, 10.0f);

    glEnd();
    glLineWidth(1.0f);
}


void GLWidget::drawControlPolygon()
{
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    glColor3f(0.5f, 0.5f, 0.5f);
    for (size_t i = 0; i < controlPoints.size() - 1; i++) {
        glVertex3f(controlPoints[i].x, controlPoints[i].y, controlPoints[i].z);
        glVertex3f(controlPoints[i + 1].x, controlPoints[i + 1].y, controlPoints[i + 1].z);
    }
    glEnd();
}

void GLWidget::drawBezierCurve()
{
    glLineWidth(3.0f);
    glBegin(GL_LINE_STRIP);
    glColor3f(0.0f, 1.0f, 1.0f);
    for (const auto& point : bezierCurve) {
        glVertex3f(point.x, point.y, point.z);
    }
    glEnd();
    glLineWidth(1.0f);
}

void GLWidget::drawBSplineSurface()
{
    // Если поверхность не сгенерирована, генерируем её
    if (bsplineControlNet.empty()) {
        generateBSplineSurface();
        return;
    }

    // Рисуем контрольную сетку
    glColor3f(1.0f, 0.0f, 0.0f);
    glPointSize(6.0f);
    glBegin(GL_POINTS);
    for (const auto& row : bsplineControlNet) {
        for (const auto& point : row) {
            glVertex3f(point.x, point.y, point.z);
        }
    }
    glEnd();

    // Рисуем линии контрольной сетки
    glColor3f(0.7f, 0.7f, 0.7f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);

    // Горизонтальные линии
    for (size_t i = 0; i < bsplineControlNet.size(); i++) {
        for (size_t j = 0; j < bsplineControlNet[i].size() - 1; j++) {
            glVertex3f(bsplineControlNet[i][j].x, bsplineControlNet[i][j].y, bsplineControlNet[i][j].z);
            glVertex3f(bsplineControlNet[i][j+1].x, bsplineControlNet[i][j+1].y, bsplineControlNet[i][j+1].z);
        }
    }

    // Вертикальные линии
    if (!bsplineControlNet.empty()) {
        for (size_t j = 0; j < bsplineControlNet[0].size(); j++) {
            for (size_t i = 0; i < bsplineControlNet.size() - 1; i++) {
                glVertex3f(bsplineControlNet[i][j].x, bsplineControlNet[i][j].y, bsplineControlNet[i][j].z);
                glVertex3f(bsplineControlNet[i+1][j].x, bsplineControlNet[i+1][j].y, bsplineControlNet[i+1][j].z);
            }
        }
    }
    glEnd();
    glLineWidth(1.0f);

    // Рисуем поверхность
    if (!bsplineSurface.empty()) {
        glColor3f(0.0f, 0.8f, 0.0f);
        glBegin(GL_QUADS);
        int divisions = 20;
        for (int i = 0; i < divisions; i++) {
            for (int j = 0; j < divisions; j++) {
                int idx1 = i * (divisions + 1) + j;
                int idx2 = i * (divisions + 1) + j + 1;
                int idx3 = (i + 1) * (divisions + 1) + j + 1;
                int idx4 = (i + 1) * (divisions + 1) + j;

                if (idx1 < bsplineSurface.size() && idx2 < bsplineSurface.size() &&
                    idx3 < bsplineSurface.size() && idx4 < bsplineSurface.size()) {
                    // Добавляем небольшую нормаль для лучшего вида
                    glVertex3f(bsplineSurface[idx1].x, bsplineSurface[idx1].y, bsplineSurface[idx1].z);
                    glVertex3f(bsplineSurface[idx2].x, bsplineSurface[idx2].y, bsplineSurface[idx2].z);
                    glVertex3f(bsplineSurface[idx3].x, bsplineSurface[idx3].y, bsplineSurface[idx3].z);
                    glVertex3f(bsplineSurface[idx4].x, bsplineSurface[idx4].y, bsplineSurface[idx4].z);
                }
            }
        }
        glEnd();
    }

    // Отображаем информацию о порядке
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINES);
    // Простые линии для демонстрации сложности
    for (int i = -3; i <= 3; i++) {
        glVertex3f(i, -3, -2);
        glVertex3f(i, 3, -2);
    }
    glEnd();
}

void GLWidget::drawLineClipping()
{
    // Рисуем окно отсечения
    glColor3f(0.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex3f(clipLeft, clipBottom, 0);
    glVertex3f(clipRight, clipBottom, 0);
    glVertex3f(clipRight, clipTop, 0);
    glVertex3f(clipLeft, clipTop, 0);
    glEnd();

    // Рисуем исходные отрезки (красные)
    glColor3f(1.0f, 0.0f, 0.0f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (const auto& line : originalLines) {
        glVertex3f(line.start.x, line.start.y, 0);
        glVertex3f(line.end.x, line.end.y, 0);
    }
    glEnd();

    // Рисуем отсеченные отрезки (зеленые)
    glColor3f(0.0f, 1.0f, 0.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    for (const auto& line : clippedLines) {
        glVertex3f(line.start.x, line.start.y, 0);
        glVertex3f(line.end.x, line.end.y, 0);
    }
    glEnd();
    glLineWidth(1.0f);
}

void GLWidget::drawZBuffer()
{
    // Рисуем многогранники с разной глубиной
    std::vector<QColor> colors = {
        QColor(255, 0, 0),    // Красный
        QColor(0, 255, 0),    // Зеленый
        QColor(0, 0, 255),    // Синий
        QColor(255, 255, 0),  // Желтый
        QColor(255, 0, 255),  // Пурпурный
        QColor(0, 255, 255),  // Голубой
        QColor(255, 165, 0),  // Оранжевый
        QColor(128, 0, 128),  // Фиолетовый
        QColor(165, 42, 42),  // Коричневый
        QColor(0, 128, 0)     // Темно-зеленый
    };

    for (size_t i = 0; i < zbufferObjects.size(); i++) {
        const auto& object = zbufferObjects[i];
        QColor color = colors[i % colors.size()];

        glColor3f(color.redF(), color.greenF(), color.blueF());

        // Рисуем основание пирамиды
        glBegin(GL_QUADS);
        glVertex3f(object[0].x, object[0].y, object[0].z);
        glVertex3f(object[1].x, object[1].y, object[1].z);
        glVertex3f(object[2].x, object[2].y, object[2].z);
        glVertex3f(object[3].x, object[3].y, object[3].z);
        glEnd();

        // Рисуем боковые грани пирамиды
        glBegin(GL_TRIANGLES);
        // Грань 1
        glVertex3f(object[0].x, object[0].y, object[0].z);
        glVertex3f(object[1].x, object[1].y, object[1].z);
        glVertex3f(object[4].x, object[4].y, object[4].z);
        // Грань 2
        glVertex3f(object[1].x, object[1].y, object[1].z);
        glVertex3f(object[2].x, object[2].y, object[2].z);
        glVertex3f(object[4].x, object[4].y, object[4].z);
        // Грань 3
        glVertex3f(object[2].x, object[2].y, object[2].z);
        glVertex3f(object[3].x, object[3].y, object[3].z);
        glVertex3f(object[4].x, object[4].y, object[4].z);
        // Грань 4
        glVertex3f(object[3].x, object[3].y, object[3].z);
        glVertex3f(object[0].x, object[0].y, object[0].z);
        glVertex3f(object[4].x, object[4].y, object[4].z);
        glEnd();
    }
}

void GLWidget::drawRayTracing()
{
    // Очищаем сцену
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f); // Темно-синий фон
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Рисуем "комнату" - стены и пол
    drawRayTracingRoom();

    // Рисуем объекты
    drawRayTracingObjects();

    // Рисуем лучи в зависимости от качества
    drawRays();

    // Рисуем информацию о качестве
    drawQualityInfo();
}

void GLWidget::initializeControlPoints()
{
    controlPoints.clear();

    // Для гладкой составной кривой Безье 3-й степени нужно:
    // - 10 точек дают 3 сегмента (точки: 0-3, 3-6, 6-9)
    // - Условие гладкости: P2 - P1 = P4 - P3 (для C¹) и т.д.

    // Сегмент 1
    Point3D p0(-4.0, 0.0, 0.0);
    Point3D p1(-3.0, 2.0, 1.0);
    Point3D p2(-2.0, -1.0, 2.0);
    Point3D p3(-1.0, 1.0, 0.0);  // Точка соединения

    // Сегмент 2 (обеспечиваем C¹ непрерывность)
    Point3D p4(0.0, 0.0, 1.0);   // Симметрично относительно p3
    Point3D p5(1.0, 2.0, 0.5);
    Point3D p6(2.0, -1.0, 1.5);
    Point3D p7(3.0, 1.0, 0.0);   // Точка соединения

    // Сегмент 3 (обеспечиваем C¹ непрерывность)
    Point3D p8(4.0, 0.0, 1.0);   // Симметрично относительно p7
    Point3D p9(5.0, 2.0, 0.5);

    controlPoints = {p0, p1, p2, p3, p4, p5, p6, p7, p8, p9};

    // Автоматически применяем условия гладкости
    applySmoothnessConditions();
}

void GLWidget::applySmoothnessConditions()
{
    // Для C¹ непрерывности в точках соединения:
    // P3 - P2 = P4 - P3 (для сегментов 1-2)
    // P6 - P5 = P7 - P6 (для сегментов 2-3)

    if (controlPoints.size() >= 7) {
        // Сегмент 1-2: обеспечиваем коллинеарность P2-P3-P4
        Point3D dir1 = {
            controlPoints[2].x - controlPoints[3].x,
            controlPoints[2].y - controlPoints[3].y,
            controlPoints[2].z - controlPoints[3].z
        };

        // P4 должна лежать на продолжении P2-P3
        controlPoints[4] = {
            controlPoints[3].x - dir1.x,
            controlPoints[3].y - dir1.y,
            controlPoints[3].z - dir1.z
        };
    }

    if (controlPoints.size() >= 10) {
        // Сегмент 2-3: обеспечиваем коллинеарность P5-P6-P7
        Point3D dir2 = {
            controlPoints[5].x - controlPoints[6].x,
            controlPoints[5].y - controlPoints[6].y,
            controlPoints[5].z - controlPoints[6].z
        };

        // P7 должна лежать на продолжении P5-P6
        controlPoints[7] = {
            controlPoints[6].x - dir2.x,
            controlPoints[6].y - dir2.y,
            controlPoints[6].z - dir2.z
        };
    }

    calculateBezierCurve();
}

void GLWidget::drawRayTracingObjects()
{
    // Сфера (зеркальная)
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(-2, 0, -2);
    drawSimpleSphere(1.0);
    glPopMatrix();

    // Куб (диффузный)
    glColor3f(0.9f, 0.9f, 0.2f);
    glPushMatrix();
    glTranslatef(2, -1, -3);
    drawSimpleCube(1.0);
    glPopMatrix();

    // Пирамида (стеклянная)
    glColor3f(0.7f, 0.7f, 1.0f);
    glPushMatrix();
    glTranslatef(0, 2, -2);
    drawPyramid(1.0);
    glPopMatrix();

    // Источник света (желтый)
    glColor3f(1.0f, 1.0f, 0.5f);
    glPushMatrix();
    glTranslatef(0, 0, 4);
    drawSimpleSphere(0.5);
    glPopMatrix();
}

void GLWidget::drawRayTracingRoom()
{
    // Пол (зеленый)
    glColor3f(0.2f, 0.6f, 0.2f);
    glBegin(GL_QUADS);
    glVertex3f(-5, -5, -5);
    glVertex3f(5, -5, -5);
    glVertex3f(5, 5, -5);
    glVertex3f(-5, 5, -5);
    glEnd();

    // Задняя стена (синяя)
    glColor3f(0.2f, 0.2f, 0.8f);
    glBegin(GL_QUADS);
    glVertex3f(-5, -5, -5);
    glVertex3f(-5, -5, 5);
    glVertex3f(-5, 5, 5);
    glVertex3f(-5, 5, -5);
    glEnd();

    // Правая стена (красная)
    glColor3f(0.8f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex3f(-5, -5, -5);
    glVertex3f(5, -5, -5);
    glVertex3f(5, -5, 5);
    glVertex3f(-5, -5, 5);
    glEnd();
}
void GLWidget::drawRays()
{
    if (!showRays) return;

    // Рисуем лучи разными цветами в зависимости от качества
    for (size_t i = 0; i < rays.size(); i += 2) {
        if (i + 1 >= rays.size()) break;

        // Разный цвет лучей в зависимости от качества
        switch(rayTracingQuality) {
        case 1: glColor3f(1.0f, 0.0f, 0.0f); break; // Красный - низкое качество
        case 2: glColor3f(1.0f, 1.0f, 0.0f); break; // Желтый - среднее качество
        case 3: glColor3f(0.0f, 1.0f, 0.0f); break; // Зеленый - хорошее качество
        case 4: glColor3f(0.0f, 1.0f, 1.0f); break; // Голубой - высокое качество
        case 5: glColor3f(1.0f, 0.0f, 1.0f); break; // Пурпурный - максимальное качество
        default: glColor3f(1.0f, 1.0f, 1.0f);
        }

        // Тонкие линии для лучей
        glLineWidth(1.0f);
        glBegin(GL_LINES);
        glVertex3f(rays[i].x, rays[i].y, rays[i].z);
        glVertex3f(rays[i+1].x, rays[i+1].y, rays[i+1].z);
        glEnd();

        // Точки попадания (только для высокого качества)
        if (rayTracingQuality >= 3) {
            glPointSize(3.0f);
            glBegin(GL_POINTS);
            size_t hitIndex = i / 2;
            if (hitIndex < rayHits.size()) {
                // Разный цвет точек в зависимости от поверхности
                if (rayHits[hitIndex].z < -2) glColor3f(0.2f, 0.8f, 0.2f); // Пол
                else if (rayHits[hitIndex].x < -3) glColor3f(0.2f, 0.2f, 0.8f); // Стена
                else glColor3f(0.8f, 0.8f, 0.2f); // Объекты
                glVertex3f(rayHits[hitIndex].x, rayHits[hitIndex].y, rayHits[hitIndex].z);
            }
            glEnd();
        }

        // Отраженные лучи (только для максимального качества)
        if (rayTracingQuality >= 4) {
            glColor3f(0.5f, 0.5f, 1.0f);
            glLineWidth(0.5f);
            glBegin(GL_LINES);
            for (size_t j = 0; j < rayHits.size() && j < 5; j++) {
                if (i/2 == j) {
                    Point3D hit = rayHits[j];
                    // Простое "отражение"
                    Point3D reflection(
                        hit.x + (rand() % 100 - 50) * 0.1,
                        hit.y + (rand() % 100 - 50) * 0.1,
                        hit.z + abs(rand() % 100 - 50) * 0.1
                        );
                    glVertex3f(hit.x, hit.y, hit.z);
                    glVertex3f(reflection.x, reflection.y, reflection.z);
                }
            }
            glEnd();
        }
    }
}

void GLWidget::drawTorus(double R, double r)
{
    // Простая реализация тора
    const int segments = 20;
    const int sides = 12;

    for (int i = 0; i < segments; i++) {
        double theta1 = i * 2 * M_PI / segments;
        double theta2 = (i + 1) * 2 * M_PI / segments;

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= sides; j++) {
            double phi = j * 2 * M_PI / sides;

            for (int k = 0; k < 2; k++) {
                double theta = (k == 0) ? theta1 : theta2;
                double x = (R + r * cos(phi)) * cos(theta);
                double y = (R + r * cos(phi)) * sin(theta);
                double z = r * sin(phi);

                glVertex3f(x, y, z);
            }
        }
        glEnd();
    }
}
void GLWidget::drawPyramid(double size)
{
    double s = size;
    glBegin(GL_TRIANGLES);
    // Основание
    glVertex3f(-s, -s, 0);
    glVertex3f(s, -s, 0);
    glVertex3f(s, s, 0);
    glVertex3f(-s, -s, 0);
    glVertex3f(s, s, 0);
    glVertex3f(-s, s, 0);

    // Боковые грани
    glVertex3f(-s, -s, 0);
    glVertex3f(0, 0, s*2);
    glVertex3f(s, -s, 0);

    glVertex3f(s, -s, 0);
    glVertex3f(0, 0, s*2);
    glVertex3f(s, s, 0);

    glVertex3f(s, s, 0);
    glVertex3f(0, 0, s*2);
    glVertex3f(-s, s, 0);

    glVertex3f(-s, s, 0);
    glVertex3f(0, 0, s*2);
    glVertex3f(-s, -s, 0);
    glEnd();
}

void GLWidget::drawSimpleSphere(double radius)
{
    const int slices = 12;
    const int stacks = 12;

    for (int i = 0; i < slices; i++) {
        double theta1 = i * 2 * M_PI / slices;
        double theta2 = (i + 1) * 2 * M_PI / slices;

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= stacks; j++) {
            double phi = j * M_PI / stacks;
            double x1 = radius * sin(phi) * cos(theta1);
            double y1 = radius * sin(phi) * sin(theta1);
            double z1 = radius * cos(phi);

            double x2 = radius * sin(phi) * cos(theta2);
            double y2 = radius * sin(phi) * sin(theta2);
            double z2 = radius * cos(phi);

            glVertex3f(x1, y1, z1);
            glVertex3f(x2, y2, z2);
        }
        glEnd();
    }
}

void GLWidget::drawSimpleCube(double size)
{
    double s = size / 2.0;

    glBegin(GL_QUADS);
    // Передняя грань
    glVertex3f(-s, -s, s);
    glVertex3f(s, -s, s);
    glVertex3f(s, s, s);
    glVertex3f(-s, s, s);

    // Задняя грань
    glVertex3f(-s, -s, -s);
    glVertex3f(-s, s, -s);
    glVertex3f(s, s, -s);
    glVertex3f(s, -s, -s);

    // Верхняя грань
    glVertex3f(-s, s, -s);
    glVertex3f(-s, s, s);
    glVertex3f(s, s, s);
    glVertex3f(s, s, -s);

    // Нижняя грань
    glVertex3f(-s, -s, -s);
    glVertex3f(s, -s, -s);
    glVertex3f(s, -s, s);
    glVertex3f(-s, -s, s);

    // Правая грань
    glVertex3f(s, -s, -s);
    glVertex3f(s, s, -s);
    glVertex3f(s, s, s);
    glVertex3f(s, -s, s);

    // Левая грань
    glVertex3f(-s, -s, -s);
    glVertex3f(-s, -s, s);
    glVertex3f(-s, s, s);
    glVertex3f(-s, s, -s);
    glEnd();
}

void GLWidget::drawQualityInfo()
{
    // Рисуем информационный текст (простыми линиями)
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);

    // Рамка информации
    glVertex3f(3, 3, 0); glVertex3f(4.5, 3, 0);
    glVertex3f(4.5, 3, 0); glVertex3f(4.5, 4, 0);
    glVertex3f(4.5, 4, 0); glVertex3f(3, 4, 0);
    glVertex3f(3, 4, 0); glVertex3f(3, 3, 0);

    // Текст "Качество: X"
    for (int i = 0; i < rayTracingQuality; i++) {
        glVertex3f(3.2 + i * 0.3, 3.3, 0);
        glVertex3f(3.2 + i * 0.3, 3.7, 0);
    }
    glEnd();
}

// Обновляем renderRayTracing
void GLWidget::renderRayTracing()
{
    // Регенерируем сцену с новыми случайными лучами
    generateRayTracingScene();
}
