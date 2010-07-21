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

#include <QPointF>
#include <QRgb>
#include <QMatrix4x4>

/**
 * Nearest neighbor interpolation.
 */
QRgb identityInterpolation(const QRgb* pixels, int width, const QPointF& point)
{
    QPoint p = point.toPoint();

    int x = p.x();
    int y = p.y();

    int pos = y * width + x;

    return pixels[pos];
}

/**
 * Bilinear interpolation.
 */
QRgb bilinearInterpolation(const QRgb* pixels, int width, const QPointF& point)
{
    QPoint p = point.toPoint();

    int x = p.x();
    int y = p.y();

    qreal dx = qAbs(point.x() - x);
    qreal dy = qAbs(point.y() - y);

    int pos00 =  y * width +  x;
    int pos01 = pos00 + width;
    int pos10 = pos00 + 1;
    int pos11 = pos01 + 1;

    QRgb rgb00 = pixels[pos00];
    QRgb rgb10 = pixels[pos10];
    QRgb rgb01 = pixels[pos01];
    QRgb rgb11 = pixels[pos11];

    qreal f00 = (1-dx)*(1-dy);
    qreal f10 =    dx *(1-dy);
    qreal f01 = (1-dx)*   dy;
    qreal f11 =    dx *   dy;

    int r = qRed  (rgb00)*f00 + qRed  (rgb10)*f10 + qRed  (rgb01)*f01 + qRed  (rgb11)*f11;
    int g = qGreen(rgb00)*f00 + qGreen(rgb10)*f10 + qGreen(rgb01)*f01 + qGreen(rgb11)*f11;
    int b = qBlue (rgb00)*f00 + qBlue (rgb10)*f10 + qBlue (rgb01)*f01 + qBlue (rgb11)*f11;

    return qRgb(r, g, b);
}

/**
 * Bicubic interpolation.
 * Adapted from http://www.paulinternet.nl/?page=bicubic [keywords = bicubic interpolation java]
 */
qreal bicubic(const QMatrix4x4& p, qreal x, qreal y) {
    qreal p00 = p(0, 0);
    qreal p01 = p(0, 1);
    qreal p02 = p(0, 2);
    qreal p03 = p(0, 3);

    qreal p10 = p(1, 0);
    qreal p11 = p(1, 1);
    qreal p12 = p(1, 2);
    qreal p13 = p(1, 3);

    qreal p20 = p(2, 0);
    qreal p21 = p(2, 1);
    qreal p22 = p(2, 2);
    qreal p23 = p(2, 3);

    qreal p30 = p(3, 0);
    qreal p31 = p(3, 1);
    qreal p32 = p(3, 2);
    qreal p33 = p(3, 3);

    qreal a00 = p11;
    qreal a01 = -.5*p10 + .5*p12;
    qreal a02 = p10 - 2.5*p11 + 2*p12 - .5*p13;
    qreal a03 = -.5*p10 + 1.5*p11 - 1.5*p12 + .5*p13;
    qreal a10 = -.5*p01 + .5*p21;
    qreal a11 = .25*p00 - .25*p02 - .25*p20 + .25*p22;
    qreal a12 = -.5*p00 + 1.25*p01 - p02 + .25*p03 + .5*p20 - 1.25*p21 + p22 - .25*p23;
    qreal a13 = .25*p00 - .75*p01 + .75*p02 - .25*p03 - .25*p20 + .75*p21 - .75*p22 + .25*p23;
    qreal a20 = p01 - 2.5*p11 + 2*p21 - .5*p31;
    qreal a21 = -.5*p00 + .5*p02 + 1.25*p10 - 1.25*p12 - p20 + p22 + .25*p30 - .25*p32;
    qreal a22 = p00 - 2.5*p01 + 2*p02 - .5*p03 - 2.5*p10 + 6.25*p11 - 5*p12 + 1.25*p13 + 2*p20 - 5*p21 + 4*p22 - p23 - .5*p30 + 1.25*p31 - p32 + .25*p33;
    qreal a23 = -.5*p00 + 1.5*p01 - 1.5*p02 + .5*p03 + 1.25*p10 - 3.75*p11 + 3.75*p12 - 1.25*p13 - p20 + 3*p21 - 3*p22 + p23 + .25*p30 - .75*p31 + .75*p32 - .25*p33;
    qreal a30 = -.5*p01 + 1.5*p11 - 1.5*p21 + .5*p31;
    qreal a31 = .25*p00 - .25*p02 - .75*p10 + .75*p12 + .75*p20 - .75*p22 - .25*p30 + .25*p32;
    qreal a32 = -.5*p00 + 1.25*p01 - p02 + .25*p03 + 1.5*p10 - 3.75*p11 + 3*p12 - .75*p13 - 1.5*p20 + 3.75*p21 - 3*p22 + .75*p23 + .5*p30 - 1.25*p31 + p32 - .25*p33;
    qreal a33 = .25*p00 - .75*p01 + .75*p02 - .25*p03 - .75*p10 + 2.25*p11 - 2.25*p12 + .75*p13 + .75*p20 - 2.25*p21 + 2.25*p22 - .75*p23 - .25*p30 + .75*p31 - .75*p32 + .25*p33;

    qreal x2 = x * x;
    qreal x3 = x2 * x;
    qreal y2 = y * y;
    qreal y3 = y2 * y;

    return a00 + a01 * y + a02 * y2 + a03 * y3 +
           a10 * x + a11 * x * y + a12 * x * y2 + a13 * x * y3 +
           a20 * x2 + a21 * x2 * y + a22 * x2 * y2 + a23 * x2 * y3 +
           a30 * x3 + a31 * x3 * y + a32 * x3 * y2 + a33 * x3 * y3;
}

QRgb bicubicInterpolation(const QRgb* pixels, int width, const QPointF& point)
{
    QMatrix4x4 mr;
    QMatrix4x4 mg;
    QMatrix4x4 mb;

    QPoint p = point.toPoint();

    int x = p.x();
    int y = p.y();

    for (int j=0; j<4; j++) {
        int jpos = (y + j - 1) * width;

        for (int i=0; i<4; i++) {
            int ipos = jpos + (x + i - 1);

            QRgb rgb = pixels[ipos];

            mr(i, j) = qRed(rgb);
            mg(i, j) = qGreen(rgb);
            mb(i, j) = qBlue(rgb);
        }
    }

    qreal dx = point.x() - x;
    qreal dy = point.y() - y;

    int r = qRound(bicubic(mr, dx, dy));
    int g = qRound(bicubic(mg, dx, dy));
    int b = qRound(bicubic(mb, dx, dy));

    return qRgb(qBound(0, r, 255), qBound(0, g, 255), qBound(0, b, 255));
}

