#ifndef POINTTABLEWIDGET_H
#define POINTTABLEWIDGET_H

#include <QTableWidget>
#include <vector>

struct Point3D;

class PointTableWidget : public QTableWidget
{
    Q_OBJECT

public:
    PointTableWidget(QWidget* parent = nullptr);
    void updatePoints(const std::vector<Point3D>& points);
    std::vector<Point3D> getPoints() const;
};

#endif // POINTTABLEWIDGET_H
