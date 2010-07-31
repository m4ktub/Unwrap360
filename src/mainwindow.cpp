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

#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopWidget>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "fullscreenexitbutton.h"
#include "settingsdialog.h"

QRgb identityInterpolation(const QRgb* pixels, int width, const QPointF& point);
QRgb bilinearInterpolation(const QRgb* pixels, int width, const QPointF& point);
QRgb bicubicInterpolation(const QRgb* pixels, int width, const QPointF& point);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_fseButton(0),
    m_settingsDialog(0)
{

    ui->setupUi(this);
    ui->sourceImage->setEmptyMessage(tr("Load a 360 degree image to start."));

    ui->zoomInButton->setIcon(QIcon::fromTheme("pdf_zoomin", QIcon(":/icons/zoomin")));
    ui->zoomOutButton->setIcon(QIcon::fromTheme("pdf_zoomout", QIcon(":/icons/zoomout")));
    ui->fullScreenButton->setIcon(QIcon::fromTheme("general_fullsize", QIcon(":/icons/fullsize")));
    ui->settingsButton->setIcon(QIcon::fromTheme("camera_camera_setting", QIcon(":/icons/settings")));
    ui->goBackButton->setIcon(QIcon::fromTheme("qgn_back_fsm", QIcon(":/icons/back")));
    ui->processButton->setIcon(QIcon::fromTheme("general_forward", QIcon(":/icons/forward")));
    ui->loadImageButton->setIcon(QIcon::fromTheme("general_toolbar_image", QIcon(":/icons/load")));
    ui->saveImageButton->setIcon(QIcon::fromTheme("notes_save", QIcon(":/icons/save")));
    ui->cancelButton->setIcon(QIcon::fromTheme("general_stop", QIcon(":/icons/stop")));

    ui->cancelButton->setVisible(false);
    ui->progressBar->setVisible(false);

    m_settingsDialog = new SettingsDialog(QApplication::desktop()->screen());
    connect(ui->sourceImage, SIGNAL(outerRadiusChanged(qreal)), m_settingsDialog, SLOT(setOuterRadius(qreal)));
    connect(ui->sourceImage, SIGNAL(innerRadiusChanged(qreal)), m_settingsDialog, SLOT(setInnerRadius(qreal)));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_settingsDialog;
}

void MainWindow::loadImage() {
    QSettings settings;
    QString dir = settings.value("defaultDir", QDir::homePath()).toString();

    QString path = QFileDialog::getOpenFileName(
            this,
            trUtf8("Choose 360º Image"),
            dir,
            tr("Image (*.jpg *.jpeg *.tif *.tiff);;Any (*.*)")
    );

    if (path.isEmpty()) {
        return;
    }

    dir = QFileInfo(path).dir().absolutePath();
    settings.setValue("defaultDir", dir);

    bool loaded = m_source.load(path);
    if (loaded) {
        setupSourceImage();
    }

    if (!loaded) {
        QMessageBox::information(QApplication::desktop(), trUtf8("Load 360º Image"), tr("Failed to load selected image."), QMessageBox::NoButton);
    }
}

void MainWindow::showSettings() {
    m_settingsDialog->exec();
}

void MainWindow::processSourceImage() {
    if (m_source.isNull()) {
        return;
    }

    ui->loadImageButton->setEnabled(false);
    ui->zoomInButton->setEnabled(false);
    ui->zoomOutButton->setEnabled(false);
    ui->settingsButton->setEnabled(false);
    ui->goBackButton->setEnabled(false);
    ui->processButton->setEnabled(false);
    ui->fullScreenButton->setEnabled(false);

    ui->action_ChooseImage->setEnabled(false);
    ui->action_SaveUnrappedImage->setEnabled(false);
    ui->action_Settings->setEnabled(false);
    ui->action_Unwrap->setEnabled(false);

    unwrap();

    ui->loadImageButton->setEnabled(true);
    ui->zoomInButton->setEnabled(true);
    ui->zoomOutButton->setEnabled(true);
    ui->settingsButton->setEnabled(true);
    ui->goBackButton->setEnabled(true);
    ui->processButton->setEnabled(true);
    ui->fullScreenButton->setEnabled(true);

    ui->action_ChooseImage->setEnabled(true);
    ui->action_Settings->setEnabled(true);
    ui->action_Unwrap->setEnabled(true);

    if (! m_result.isNull()) {
        ui->sourceImage->setShowCircles(false);
        ui->sourceImage->setImage(m_result);
        ui->saveImageButton->setEnabled(true);

        ui->action_SaveUnrappedImage->setEnabled(true);
    }
}

void MainWindow::unwrap()
{
    SettingsDialog::ImageInterpolation interpolation = m_settingsDialog->interpolation();
    bool invert = m_settingsDialog->invertFinalImage();

    int sourceWidth = m_source.width();
    int height = m_settingsDialog->resultHeight();
    int width  = m_settingsDialog->resultWidth();
    int pixels = height * width;
    QImage output = QImage(width, height, m_source.format());

    const QRgb* sourcePixels = (const QRgb*) m_source.bits();
    QRgb* outputPixels = (QRgb*) output.bits();

    QPointF center = ui->sourceImage->center();
    qreal innerRadius = ui->sourceImage->innerRadius();
    qreal outerRadius = ui->sourceImage->outerRadius();

    ui->cancelButton->setVisible(true);
    ui->progressBar->setVisible(true);

    QTransform transform;

    m_result = QImage();
    m_cancel = false;

    QPointF referencePoint(1, 0);

    for (int y = 0; !m_cancel && y < height; y++) {
        int usedY = invert ? height - (y + 1) : y;
        float ro = innerRadius + ((usedY * (outerRadius - innerRadius))  / height);

        int offset = y * width;

        for (int x = 0; !m_cancel && x < width; x++) {
            float ang = (360.0 * x) / width;

            transform.reset();
            transform.scale(ro, ro);
            transform.rotate(-ang); // mirrors reflect

            QPointF point = transform.map(referencePoint) + center;
            QRgb rgb;

            switch (interpolation) {
            case SettingsDialog::NoInterpolation:
                rgb = identityInterpolation(sourcePixels, sourceWidth, point);
                break;
            case SettingsDialog::BilinearInterpolation:
                rgb = bilinearInterpolation(sourcePixels, sourceWidth, point);
                break;
            case SettingsDialog::BicubicInterpolation:
                rgb = bicubicInterpolation(sourcePixels, sourceWidth, point);
                break;
            default:
                rgb = qRgb(0, 0, 0);
            }

            outputPixels[offset++] = rgb;

            int progress = 100.0 * ((float) offset) / pixels;
            ui->progressBar->setValue(progress);
        }
    }

    if (!m_cancel) {
        int finalWidth = m_settingsDialog->finalWidth();
        int finalHeight = m_settingsDialog->finalHeight();

        qreal factor = ((float) (m_settingsDialog->equiRectangular() ? m_settingsDialog->fov() : 180)) / 180.0;
        int scaledWidth = finalWidth;
        int scaledHeight = finalHeight * factor;

        QImage finalScaled = output.scaled(QSize(scaledWidth, scaledHeight), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        m_result = QImage(QSize(finalWidth, finalHeight), output.format());

        QPainter p(&m_result);
        p.fillRect(m_result.rect(), m_settingsDialog->equiRectangularFillColor());
        p.drawImage(QPoint(0, (finalHeight - scaledHeight) / 2), finalScaled);
    }

    ui->cancelButton->setVisible(false);
    ui->progressBar->setVisible(false);
}

void MainWindow::enterFullScreen() {
    bool isFullScreen = windowState() & Qt::WindowFullScreen;

    if (isFullScreen) {
        return;
    }

    ui->buttonFrame->setVisible(false);

    if (!m_fseButton) {
        m_fseButton = new FullScreenExitButton(this);
        m_fseButton->setIcon(QIcon::fromTheme(QLatin1String("general_fullsize"), QIcon(":/icons/fullsize")));
    }

    showFullScreen();
}

void MainWindow::saveResultImage() {
    if (m_result.isNull()) {
        return;
    }

    QSettings settings;
    QString dir = settings.value("defaultSaveDir", QDir::homePath()).toString();

    QString path = QFileDialog::getSaveFileName(
            this,
            tr("Choose File to Save"),
            dir);

    if (path.isEmpty()) {
        return;
    }

    QFileInfo file(path);

    dir = file.dir().absolutePath();
    settings.setValue("defaultSaveDir", dir);

    if (file.suffix().isEmpty()) {
        path = path.append(".jpg");
    }

    bool saved = m_result.save(path, 0, 90);
    if (! saved) {
        QMessageBox::information(QApplication::desktop(), trUtf8("Load 360º Image"), tr("Failed to save image in the specified location."), QMessageBox::NoButton);
    }
}

void MainWindow::toggleFullScreen() {
     bool isFullScreen = windowState() & Qt::WindowFullScreen;

     if (isFullScreen) {
         showNormal();
     }
     else {
         showFullScreen();
     }
}

bool MainWindow::event(QEvent *event) {
    bool isFullScreen = windowState() & Qt::WindowFullScreen;

    switch (event->type()) {
    case QEvent::WindowStateChange:
        if (!isFullScreen) {
            ui->buttonFrame->setVisible(true);
        }

        break;
    default:
        break;
    }

    return QMainWindow::event(event);
}

void MainWindow::cancelProcessing()
{
    m_cancel = true;
}

void MainWindow::setupSourceImage()
{
    ui->sourceImage->setShowCircles(true);
    ui->sourceImage->setImage(m_source);
    m_result = QImage();

    ui->zoomInButton->setEnabled(true);
    ui->zoomOutButton->setEnabled(true);
    ui->settingsButton->setEnabled(true);
    ui->goBackButton->setEnabled(false);
    ui->processButton->setEnabled(true);
    ui->saveImageButton->setEnabled(false);

    ui->action_Settings->setEnabled(true);
    ui->action_Unwrap->setEnabled(true);
}
