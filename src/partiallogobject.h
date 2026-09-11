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

#ifndef PARTIALLOGOBJECT_H
#define PARTIALLOGOBJECT_H

#include "ui_partiallogobject.h"

#include <QMessageBox>
#include <QWidget>
#include "highlighter.h"

namespace Ui {
class partialLogObject;
}

class partialLogObject : public QWidget {
    Q_OBJECT

public:
    explicit partialLogObject(QWidget* parent = 0, QString logText = "", bool higlighterDisabled = false);
    ~partialLogObject() = default;
    QString getLogText();

private:
    Ui::partialLogObject m_ui;
    highlighter* m_logHighlighter;
    int m_start{0};
    int m_errorStart{0};
    long m_infectedStart{0};

    void setLogText(QString);

private slots:
    void slot_searchButtonClicked();
    void slot_searchLineEditChanged();
    void slot_clearLineEditButtonClicked();
    void slot_totalErrorButtonClicked();
    void slot_infectedFilesButtonClicked();
    void slot_add_remove_highlighter(bool state);
};

#endif  // PARTIALLOGOBJECT_H
