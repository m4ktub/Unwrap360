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

#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QVector2D>
#include <QSettings>

#include "imagearea.h"
#include "imagemarker.h"

#include <QDebug>

ImageArea::ImageArea(QWidget *parent) :
    QGraphicsView(parent),
    m_showCircles(true),
    m_frame(0), m_centerMarker(0),
    m_innerMarker(0), m_outerMarker(0),
    m_innerCircle(0), m_outerCircle(0)
{
    setCacheMode(CacheBackground);
    setTransformationAnchor(AnchorUnderMouse);
    setDragMode(ScrollHandDrag);

    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);

    setScene(scene);
}

ImageArea::~ImageArea()
{
    saveSettings();
}

void ImageArea::setEmptyMessage(const QString &message) {
    m_message = message;

    if (scene()->items().empty()) {
        scene()->addText(message)->setDefaultTextColor(palette().color(QPalette::ButtonText));
    }
}

void ImageArea::setImage(const QImage &image) {
    saveSettings();
    clearScene();

    m_pixmap = QPixmap::fromImage(image);
    if (m_pixmap.isNull()) {
        return;
    }

    scene()->setSceneRect(m_pixmap.rect());
    fitInView(m_pixmap.rect(), Qt::KeepAspectRatio);    

    setupMarkers();
}

void ImageArea::clearScene()
{
    if (m_frame) {
        scene()->removeItem(m_frame);
        delete m_frame;
    }

    if (m_centerMarker) {
        delete m_centerMarker;
    }

    foreach(QGraphicsItem *item, scene()->items()) {
        delete item;
    }

    m_frame = 0;
    m_centerMarker = 0;
    m_innerMarker = 0;
    m_innerCircle = 0;
    m_outerMarker = 0;
    m_outerCircle = 0;
}

void ImageArea::setupMarkers()
{
    if (!m_showCircles) {
        return;
    }

    m_frame = scene()->addRect(m_pixmap.rect());
    m_frame->setPen(QPen(QBrush(), 0, Qt::NoPen));

    QSettings settings;

    QPoint innerPoint = settings.value(settingsKey("inner"), QPoint(-200, 0)).toPoint();
    QPoint outerPoint = settings.value(settingsKey("outer"), QPoint(200, 0)).toPoint();
    QPoint centerPoint = settings.value(settingsKey("center"), QPoint(0, 0)).toPoint();

    m_innerMarker  = createMarker(innerPoint.x(), innerPoint.y(),  true, Qt::green);
    m_outerMarker  = createMarker(outerPoint.x(), outerPoint.y(),  true, Qt::red);
    m_centerMarker = createMarker(centerPoint.x(), centerPoint.y(), false, Qt::blue);

    m_innerMarker->setLimitMarker(m_outerMarker, true);
    m_outerMarker->setLimitMarker(m_innerMarker, false);

    setEmptyMessage(m_message);
    updateCircles();
}

ImageMarker* ImageArea::createMarker(int x, int y, bool inFrame, const QColor &color)
{
    int s = 100;

    int cx = m_pixmap.rect().center().x();
    int cy = m_pixmap.rect().center().y();

    ImageMarker* marker = new ImageMarker(this);
    marker->setRect(cx - (s / 2), cy - (s / 2), s, s);
    marker->setPen(QPen(color));

    if (inFrame) {
        marker->setParentItem(m_frame);
        marker->setPos(x, y);
    }
    else {
        scene()->addItem(marker);
        marker->setPos(x, y);
    }

    return marker;
}

void ImageArea::zoomIn() {
    scale(qreal(1.2), qreal(1.2));
}

void ImageArea::zoomOut() {
    scale(1 / qreal(1.2), 1 / qreal(1.2));
}

void ImageArea::drawBackground(QPainter *painter, const QRectF &rect) {
    painter->fillRect(rect, palette().color(QPalette::Background));
    painter->drawPixmap(rect, m_pixmap, rect);
}

void ImageArea::updateCircles() {
    if (!m_centerMarker) {
        return;
    }

    if (m_outerCircle) {
        delete m_outerCircle;
    }

    if (m_innerCircle) {
        delete m_innerCircle;
    }

    QRect pixmapRect = m_pixmap.rect();
    QPoint pixmapCenter = pixmapRect.center();

    m_center = m_centerMarker->scenePos() + m_pixmap.rect().center();

    qreal innerRadius = QVector2D(m_innerMarker->pos()).length();
    if (innerRadius != m_innerRadius) {
        m_innerRadius = innerRadius;
        emit innerRadiusChanged(m_innerRadius);
    }
    else {
        m_innerRadius = innerRadius;
    }

    qreal outerRadius = QVector2D(m_outerMarker->pos()).length();
    if (outerRadius != m_outerRadius) {
        m_outerRadius = outerRadius;
        emit outerRadiusChanged(m_outerRadius);
    }
    else {
        m_outerRadius = outerRadius;
    }

    m_frame->setPos(m_center - pixmapCenter);

    m_innerCircle = new QGraphicsEllipseItem();
    m_innerCircle->setRect(-m_innerRadius, -m_innerRadius, m_innerRadius*2, m_innerRadius*2);
    m_innerCircle->setPen(QPen(Qt::green));
    m_innerCircle->setParentItem(m_frame);
    m_innerCircle->setPos(pixmapCenter.x(), pixmapCenter.y());

    m_outerCircle = new QGraphicsEllipseItem();
    m_outerCircle->setRect(-m_outerRadius, -m_outerRadius, m_outerRadius*2, m_outerRadius*2);
    m_outerCircle->setPen(QPen(Qt::red));
    m_outerCircle->setParentItem(m_frame);
    m_outerCircle->setPos(pixmapCenter.x(), pixmapCenter.y());
}

QString ImageArea::settingsKey(const QString& param) {
    return QString("ImageArea360/size_%1x%2/%3")
            .arg(m_pixmap.size().width())
            .arg(m_pixmap.size().height())
            .arg(param);
}

void ImageArea::saveSettings() {
    if (m_pixmap.isNull()) {
        return;
    }

    if (!m_innerMarker || !m_outerMarker || !m_centerMarker) {
        return;
    }

    QSettings settings;

    settings.setValue(settingsKey("inner"), m_innerMarker->pos().toPoint());
    settings.setValue(settingsKey("outer"), m_outerMarker->pos().toPoint());
    settings.setValue(settingsKey("center"), m_centerMarker->pos().toPoint());
}

QImage ImageArea::image() const
{
    return m_pixmap.toImage();
}

QPointF ImageArea::center() const
{
    return m_center;
}

qreal ImageArea::innerRadius() const
{
    return m_innerRadius;
}

qreal ImageArea::outerRadius() const
{
    return m_outerRadius;
}

bool ImageArea::showCircles()
{
    return m_showCircles;
}

void ImageArea::setShowCircles(bool show)
{
    m_showCircles = show;
}
