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

#include <QImage>
#include <QPointF>
#include <QRgb>
#include <QGenericMatrix>
#include <QMatrix2x2>
#include <QMatrix4x4>

QRgb identityInterpolation(const QImage& image, float ang, float ro, const QPointF& point)
{
    Q_UNUSED(ang);
    Q_UNUSED(ro);

    return image.pixel(point.toPoint());
}

QRgb bilinearInterpolation_avg(const QImage& image, float ang, float ro, const QPointF& point0)
{
    Q_UNUSED(ro);

    QTransform transform;
    transform.rotate(ang);

    QPointF point1 = transform.map(QPointF( 1,  0)) + point0;
    QPointF point2 = transform.map(QPointF(-1,  0)) + point0;
    QPointF point3 = transform.map(QPointF( 0,  1)) + point0;
    QPointF point4 = transform.map(QPointF( 0, -1)) + point0;
    QPointF point5 = transform.map(QPointF( 1,  1)) + point0;
    QPointF point6 = transform.map(QPointF( 1, -1)) + point0;
    QPointF point7 = transform.map(QPointF(-1,  1)) + point0;
    QPointF point8 = transform.map(QPointF(-1,  1)) + point0;

    QRgb rgb0 = image.pixel(point0.toPoint());
    QRgb rgb1 = image.pixel(point1.toPoint());
    QRgb rgb2 = image.pixel(point2.toPoint());
    QRgb rgb3 = image.pixel(point3.toPoint());
    QRgb rgb4 = image.pixel(point4.toPoint());
    QRgb rgb5 = image.pixel(point5.toPoint());
    QRgb rgb6 = image.pixel(point6.toPoint());
    QRgb rgb7 = image.pixel(point7.toPoint());
    QRgb rgb8 = image.pixel(point8.toPoint());

    int r = (qRed(rgb0) + qRed(rgb1) + qRed(rgb2) + qRed(rgb3) + qRed(rgb4) + qRed(rgb5) + qRed(rgb6) + qRed(rgb7) + qRed(rgb8)) / 9;
    int g = (qGreen(rgb0) + qGreen(rgb1) + qGreen(rgb2) + qGreen(rgb3) + qGreen(rgb4) + qGreen(rgb5) + qGreen(rgb6) + qGreen(rgb7) + qGreen(rgb8)) / 9;
    int b = (qBlue(rgb0) + qBlue(rgb1) + qBlue(rgb2) + qBlue(rgb3) + qBlue(rgb4) + qBlue(rgb5) + qBlue(rgb6) + qBlue(rgb7) + qBlue(rgb8)) / 9;

    return qRgb(r, g, b);
}

QRgb bilinearInterpolation_mx(const QImage& image, float ang, float ro, const QPointF& point)
{
    Q_UNUSED(ang);
    Q_UNUSED(ro);

    int x = (int) point.x();
    int y = (int) point.y();

    qreal dx = point.x() - x;
    qreal dy = point.y() - y;

    QGenericMatrix<2, 1, qreal> mfx;
    QGenericMatrix<1, 2, qreal> mfy;

    mfx(0, 0) = 1 - dx;
    mfx(0, 1) =     dx;
    mfy(0, 0) = 1 - dy;
    mfy(1, 0) =     dy;

    QMatrix2x2 mr;
    QMatrix2x2 mg;
    QMatrix2x2 mb;

    for (int i=0; i<2; i++) {
        for (int j=0; j<2; j++) {
            QPoint p(x + j, y + i);
            QRgb rgb = image.pixel(p);

            // mx is transposed
            mr(j, i) = qRed(rgb);
            mg(j, i) = qGreen(rgb);
            mb(j, i) = qBlue(rgb);
        }
    }

    int r = (mfx * mr * mfy)(0, 0);
    int g = (mfx * mg * mfy)(0, 0);
    int b = (mfx * mb * mfy)(0, 0);

    return qRgb(r, g, b);
}

QRgb bilinearInterpolation(const QImage& image, float ang, float ro, const QPointF& point0)
{
    Q_UNUSED(ang);
    Q_UNUSED(ro);

    int x = (int) point0.x();
    int y = (int) point0.y();

    qreal dx = point0.x() - x;
    qreal dy = point0.y() - y;

    QPoint p00((int) x,     (int) y    );
    QPoint p01((int) x    , (int) y + 1);
    QPoint p10((int) x + 1, (int) y    );
    QPoint p11((int) x + 1, (int) y + 1);

    QRgb rgb00 = image.pixel(p00);
    QRgb rgb01 = image.pixel(p01);
    QRgb rgb10 = image.pixel(p10);
    QRgb rgb11 = image.pixel(p11);

    qreal f00 = (1-dx)*(1-dy);
    qreal f10 =    dx *(1-dy);
    qreal f01 = (1-dx)*   dy;
    qreal f11 =    dx *   dy;

    int r = qRed  (rgb00)*f00 + qRed  (rgb10)*f10 + qRed  (rgb01)*f01 + qRed  (rgb11)*f11;
    int g = qGreen(rgb00)*f00 + qGreen(rgb10)*f10 + qGreen(rgb01)*f01 + qGreen(rgb11)*f11;
    int b = qBlue (rgb00)*f00 + qBlue (rgb10)*f10 + qBlue (rgb01)*f01 + qBlue (rgb11)*f11;

    return qRgb(r, g, b);
}

float bicubic(QMatrix4x4 p, float x,float y, float x2,float y2, float x3, float y3) {
  int p00 = p(0, 0);
  int p10 = p(1, 0);
  int p20 = p(2, 0);
  int p30 = p(3, 0);

  int p01 = p(0, 1);
  int p11 = p(1, 1);
  int p21 = p(2, 1);
  int p31 = p(3, 1);

  int p02 = p(0, 2);
  int p12 = p(1, 2);
  int p22 = p(2, 2);
  int p32 = p(3, 2);

  int p03 = p(0, 3);
  int p13 = p(1, 3);
  int p23 = p(2, 3);
  int p33 = p(3, 3);

  int a00 =    p11;
  int a01 =   -p10 +   p12;
  int a02 =  2*p10 - 2*p11 +   p12 -   p13;
  int a03 =   -p10 +   p11 -   p12 +   p13;
  int a10 =   -p01 +   p21;
  int a11 =    p00 -   p02 -   p20 +   p22;
  int a12 = -2*p00 + 2*p01 -   p02 +   p03 + 2*p20 - 2*p21 +   p22 -   p23;
  int a13 =    p00 -   p01 +   p02 -   p03 -   p20 +   p21 -   p22 +   p23;
  int a20 =  2*p01 - 2*p11 +   p21 -   p31;
  int a21 = -2*p00 + 2*p02 + 2*p10 - 2*p12 -   p20 +   p22 +   p30 -   p32;
  int a22 =  4*p00 - 4*p01 + 2*p02 - 2*p03 - 4*p10 + 4*p11 - 2*p12 + 2*p13 + 2*p20 - 2*p21 + p22 - p23 - 2*p30 + 2*p31 - p32 + p33;
  int a23 = -2*p00 + 2*p01 - 2*p02 + 2*p03 + 2*p10 - 2*p11 + 2*p12 - 2*p13 -   p20 +   p21 - p22 + p23 +   p30 -   p31 + p32 - p33;
  int a30 =   -p01 +   p11 -   p21 +   p31;
  int a31 =    p00 -   p02 -   p10 +   p12 +   p20 -   p22 -   p30 +   p32;
  int a32 = -2*p00 + 2*p01 -   p02 +   p03 + 2*p10 - 2*p11 +   p12 -   p13 - 2*p20 + 2*p21 - p22 + p23 + 2*p30 - 2*p31 + p32 - p33;
  int a33 =    p00 -   p01 +   p02 -   p03 -   p10 +   p11 -   p12 +   p13 +   p20 -   p21 + p22 - p23 -   p30 +   p31 - p32 + p33;

  return
    a00      + a01 * y      + a02 * y2      + a03 * y3 +
    a10 * x  + a11 * x  * y + a12 * x  * y2 + a13 * x  * y3 +
    a20 * x2 + a21 * x2 * y + a22 * x2 * y2 + a23 * x2 * y3 +
    a30 * x3 + a31 * x3 * y + a32 * x3 * y2 + a33 * x3 * y3;
}

QRgb bicubicInterpolation(const QImage& image, float ang, float ro, const QPointF& point0)
{
    Q_UNUSED(ro);

    QTransform transform;
    transform.rotate(ang);

    QMatrix4x4 mr;
    QMatrix4x4 mg;
    QMatrix4x4 mb;

    for(int y=0; y<4; y++) {
        for(int x=0; x<4; x++) {
            QPointF point = transform.map(QPointF(x-2,  y-2)) + point0;

            QRgb rgb = image.pixel(point.toPoint());

            mr(x, y) = qRed(rgb);
            mg(x, y) = qGreen(rgb);
            mb(x, y) = qBlue(rgb);
        }
    }

    QPointF dp = point0 - point0.toPoint();

    float dx = dp.x();
    float dy = dp.y();
    float dx2 = dx * dx;
    float dx3 = dx2 * dx;
    float dy2 = dy * dy;
    float dy3 = dy2 * dy;

    float red   = bicubic(mr, dx, dy, dx2, dy2, dx3, dy3);
    float green = bicubic(mg, dx, dy, dx2, dy2, dx3, dy3);
    float blue  = bicubic(mb, dx, dy, dx2, dy2, dx3, dy3);

    return qRgb(qMin(255, qRound(red)), qMin(255, qRound(green)), qMin(255, qRound(blue)));
}
