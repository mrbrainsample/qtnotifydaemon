/*
 * Copyright (C) 2011 Drogolov Roman Olegovich <drull@drull.org.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <http://www.gnu.org/licenses/>
 *
 */



#include <QLabel>
#include <QMouseEvent>
#include <QPainter>

class QClickLabel : public QLabel
{
	Q_OBJECT
	public:
QClickLabel(QWidget *parent = 0) : QLabel(parent){};
	signals:
void clicked();
	protected:
void mouseReleaseEvent(QMouseEvent *e)
	{
	if(e->button() == Qt::LeftButton || e->button() == Qt::RightButton)
		{
		emit clicked();
		}
	}
};
