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

#include <QStyleOptionGraphicsItem>
#include <QVector2D>

#include <QDebug>

#include "imagemarker.h"
#include "imagearea.h"

ImageMarker::ImageMarker(ImageArea *view) :
        m_view(view), m_limit(0)
{
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
}

QVariant ImageMarker::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && scene()) {
        QPointF newPos = getLimitPos(value.toPointF());

        if (! newPos.isNull()) {
            return newPos;
        }
    }
    else if (change == ItemPositionHasChanged && scene()) {
        m_view->updateCircles();
    }

    return QGraphicsItem::itemChange(change, value);
}

QPointF ImageMarker::getLimitPos(QPointF newPos)
{
    qreal loLen;
    qreal hiLen;

    if (!m_limit) {
        return QPointF();
    }

    QVector2D ownVector = QVector2D(newPos);
    QVector2D limVector = QVector2D(m_limit->pos());

    if (m_outerLimit) {
        loLen = ownVector.length();
        hiLen = limVector.length();
    }
    else {
        loLen = limVector.length();
        hiLen = ownVector.length();
    }

    if (loLen <= hiLen) {
        return QPointF();
    }

    return (ownVector.normalized() * (m_outerLimit ? hiLen : loLen)).toPointF();
}

void ImageMarker::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(pen());

    if (option->state & QStyle::State_Sunken) {
        painter->setBrush(QBrush(pen().color(), Qt::SolidPattern));
        painter->drawEllipse(boundingRect());
    }
    else {
        painter->drawArc(boundingRect(), 0, 5760);
    }

}

void ImageMarker::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    QGraphicsItem::mousePressEvent(event);
}

void ImageMarker::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    QGraphicsItem::mouseReleaseEvent(event);
}

void ImageMarker::setLimitMarker(ImageMarker* limit, bool outer)
{
    m_outerLimit = outer;
    m_limit = limit;
}
