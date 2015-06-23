/*

 Copyright (C) 2004, Aseem Agarwala, roto@agarwala.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or (at
 your option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 USA

 */
#include "ui_RangeDialog.h"
#include "RangeDialog.h"
#include <QMessageBox>

RangeDialog::RangeDialog(QWidget *parent, int lowVal, int highVal)
    : QDialog(parent), ui(new Ui::RangeDialog), _lowVal(lowVal), _highVal(highVal)
{
    ui->setupUi(this);

    QString num;
    num.setNum(highVal);

    int maxLength = num.size();
    ui->highEdit->setText(num);
    ui->highEdit->setMaxLength(maxLength);

    num.setNum(lowVal);
    ui->lowEdit->setText(num);
    ui->lowEdit->setMaxLength(maxLength);

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(tryToAccept()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

RangeDialog::~RangeDialog()
{
    delete ui;
}

void RangeDialog::tryToAccept()
{
    Vec2i result = getRange();
    if (result.x() >= _lowVal && result.y() <= _highVal)
    {
        accept();
    }
    else if(result.x() < _lowVal)
        QMessageBox::warning(this, tr("RangeDialog"),
                             tr("Start frame %1 is lower than %2!!")
                             .arg(result.x()).arg(_lowVal));
    else
        QMessageBox::warning(this, tr("RangeDialog"),
                             tr("End frame %1 is higher than %2!!")
                             .arg(result.y()).arg(_highVal));
}

Vec2i RangeDialog::getRange() const
{
    bool ok;
    Vec2i result;
    QString lowEditText = ui->lowEdit->text();
    result.set_x(lowEditText.toInt(&ok));
    assert(ok);
    QString highEditText = ui->highEdit->text();
    result.set_y(highEditText.toInt(&ok));
    assert(ok);
    return result;
}
