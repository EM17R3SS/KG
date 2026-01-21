#include "pointtablewidget.h"
#include "glwidget.h"
#include <QHeaderView>

PointTableWidget::PointTableWidget(QWidget* parent) : QTableWidget(parent)
{
    setColumnCount(3);
    setHorizontalHeaderLabels({"X", "Y", "Z"});
    verticalHeader()->setVisible(true);
}

void PointTableWidget::updatePoints(const std::vector<Point3D>& points)
{
    setRowCount(points.size());
    for (size_t i = 0; i < points.size(); i++) {
        setItem(i, 0, new QTableWidgetItem(QString::number(points[i].x, 'f', 2)));
        setItem(i, 1, new QTableWidgetItem(QString::number(points[i].y, 'f', 2)));
        setItem(i, 2, new QTableWidgetItem(QString::number(points[i].z, 'f', 2)));
    }
}

std::vector<Point3D> PointTableWidget::getPoints() const
{
    std::vector<Point3D> points;
    for (int i = 0; i < rowCount(); i++) {
        double x = item(i, 0)->text().toDouble();
        double y = item(i, 1)->text().toDouble();
        double z = item(i, 2)->text().toDouble();
        points.push_back(Point3D(x, y, z));
    }
    return points;
}
