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

#include <QDebug>
#include <QRadialGradient>

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#define PI 3.14159265358979323846

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    connect(this, SIGNAL(accepted()), SLOT(saveValues()));
    connect(this, SIGNAL(rejected()), SLOT(loadValues()));

    loadValues();
}

SettingsDialog::~SettingsDialog()
{
    saveValues();
    delete ui;
}

int SettingsDialog::fov()
{
    return ui->fovSpinBox->value();
}

int SettingsDialog::focalPointPercent()
{
    return ui->focalPointSpinBox->value();
}

int SettingsDialog::resultWidth()
{
    qreal focalRadius = m_innerRadius + ((focalPointPercent() / 100.0) * (m_outerRadius - m_innerRadius));
    return 2 * focalRadius * PI;
}

int SettingsDialog::resultHeight()
{
    return (resultWidth() * fov()) / 360; // width = 360 deg
}

int SettingsDialog::finalWidth()
{
    return ui->finalWidthSpinBox->value();
}

int SettingsDialog::finalHeight()
{
    return ui->finalHeightSpinBox->value();
}

bool SettingsDialog::invertFinalImage()
{
    return ui->skyUpCheckbox->isChecked();
}

SettingsDialog::ImageInterpolation SettingsDialog::interpolation()
{
    return (ImageInterpolation) ui->interpolationComboBox->currentIndex();
}

void SettingsDialog::updateSizeLabel()
{
    ui->imageWidthSizeLabel->setText(QString("%1x%2").arg(resultWidth()).arg(resultHeight()));
}

void SettingsDialog::updateFinalHeightRatio()
{
    int height = (ui->finalWidthSpinBox->value() * fov()) / 360;
    ui->finalHeightSpinBox->blockSignals(true);
    ui->finalHeightSpinBox->setValue(height);
    ui->finalHeightSpinBox->blockSignals(false);
}

void SettingsDialog::updateFinalWidthRatio()
{
    int width = (ui->finalHeightSpinBox->value() * 360) / fov();
    ui->finalWidthSpinBox->blockSignals(true);
    ui->finalWidthSpinBox->setValue(width);
    ui->finalWidthSpinBox->blockSignals(false);
}

void SettingsDialog::setInnerRadius(qreal radius)
{
    m_innerRadius = radius;

    if (m_outerRadius != 0) {
        ui->focalPointSpinBox->setMinimum((m_innerRadius * 100) / m_outerRadius);
    }

    updateSizeLabel();
}

void SettingsDialog::setOuterRadius(qreal radius)
{
    m_outerRadius = radius;

    if (m_outerRadius != 0) {
        ui->focalPointSpinBox->setMinimum((m_innerRadius * 100) / m_outerRadius);
    }

    updateSizeLabel();
}

void SettingsDialog::loadValues()
{
    m_settings.beginGroup("Processing");
    ui->fovSpinBox->setValue(m_settings.value("fov", fov()).toInt());
    ui->focalPointSpinBox->setValue(m_settings.value("focalPercent", focalPointPercent()).toInt());
    ui->interpolationComboBox->setCurrentIndex(m_settings.value("interpolationOption", interpolation()).toInt());
    ui->finalHeightSpinBox->setValue(m_settings.value("finalHeight", finalHeight()).toInt());
    ui->finalWidthSpinBox->setValue(m_settings.value("finalWidth", finalWidth()).toInt());
    ui->skyUpCheckbox->setChecked(m_settings.value("invert", invertFinalImage()).toBool());
    m_settings.endGroup();
}

void SettingsDialog::saveValues()
{
    m_settings.beginGroup("Processing");
    m_settings.setValue("fov", fov());
    m_settings.setValue("focalPercent", focalPointPercent());
    m_settings.setValue("interpolationOption", (int) interpolation());
    m_settings.setValue("finalHeight", finalHeight());
    m_settings.setValue("finalWidth", finalWidth());
    m_settings.setValue("invert", invertFinalImage());
    m_settings.endGroup();
}
