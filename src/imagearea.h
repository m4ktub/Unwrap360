/******************************************************************************
 *
 * Copyright (c) 2010 Cláudio F. Gil <claudio.f.gil@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *****************************************************************************/

#ifndef IMAGEAREA_H
#define IMAGEAREA_H

#include <QGraphicsView>
#include <QGraphicsItem>

class ImageMarker;

class ImageArea : public QGraphicsView
{
    Q_OBJECT

public:
    explicit ImageArea(QWidget *parent = 0);
    ~ImageArea();

    QImage image() const;
    void setImage(const QImage &image);

    bool showCircles();
    void setShowCircles(bool show);

    QPointF center() const;
    qreal innerRadius() const;
    qreal outerRadius() const;

public slots:
    void setEmptyMessage(const QString& message);

    void updateCircles();
    void zoomIn();
    void zoomOut();

signals:
    void innerRadiusChanged(qreal radius);
    void outerRadiusChanged(qreal radius);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect);

private:
    QString m_message;
    QPixmap m_pixmap;

    bool m_showCircles;
    QPointF m_center;
    qreal m_innerRadius;
    qreal m_outerRadius;

    QGraphicsRectItem* m_frame;
    QGraphicsItem* m_centerMarker;
    ImageMarker* m_innerMarker;
    ImageMarker* m_outerMarker;
    QGraphicsEllipseItem* m_innerCircle;
    QGraphicsEllipseItem* m_outerCircle;

    void clearScene();
    void setupMarkers();
    void updatePixmap();
    QString settingsKey(const QString& param);
    void saveSettings();
    ImageMarker* createMarker(int x, int y, bool inFrame, const QColor& color);
};

#endif // IMAGEAREA_H
