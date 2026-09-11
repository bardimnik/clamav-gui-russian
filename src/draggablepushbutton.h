/***************************************************************************
 *   Copyright (C) 2015 by Joerg Zopes                                     *
 *   joerg.zopes@gmx.de                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef DRAGGABLEPUSHBUTTON_H
#define DRAGGABLEPUSHBUTTON_H

#include "ui_draggablepushbutton.h"

#include <QPushButton>
#include <QMimeData>
#include <QUrl>
#include <QDrag>

namespace Ui {
class draggablePushButton;
}

class draggablePushButton : public QPushButton
{
    Q_OBJECT

public:
    explicit draggablePushButton(QWidget *parent = 0);
    draggablePushButton(QIcon, QString, QWidget *, QString);
    ~draggablePushButton() = default;
    void mouseMoveEvent(QMouseEvent *);

private:
    Ui::draggablePushButton m_ui;
    QString m_dragDropText;

signals:
    void dragStarted();

};

#endif // DRAGGABLEPUSHBUTTON_H
