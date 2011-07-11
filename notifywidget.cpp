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




#include <QApplication>
#include <QBitmap>
#include <QPixmap>
#include <vector>
#include "notifywidget.h"
#include "dbus.h"
#include "qclicklabel.h"
#include "notifyarea.h"
///////////////////////////////////////////////////////////////

#define FADE_TIMER_TIMEOUT 20




NotifyWidget::NotifyWidget(const char *name, std::vector<Message> *msg, NotifyArea *p)
{
strcpy(widgetName,name);
messageStack = msg;
parent = p;

text = new QClickLabel(this);
timer = new QTimer(this);
icon = new QClickLabel(this);
fadeTimer = new QTimer();

connect(timer, SIGNAL(timeout()), this, SLOT(hideWidget()));
connect(fadeTimer, SIGNAL(timeout()), this, SLOT(fadeWidget()));
connect(text, SIGNAL(clicked()), this,SLOT(hideWidget()));
connect(icon, SIGNAL(clicked()), this,SLOT(hideWidget()));

this->setWindowFlags(Qt::SplashScreen | Qt::WindowStaysOnTopHint);
text->setAttribute(Qt::WA_NoSystemBackground);
icon->setAttribute(Qt::WA_NoSystemBackground);



}
///////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////


void NotifyWidget::fadeWidget()
{
if(this->windowOpacity()>0 && this->isVisible())
	{
	this->setWindowOpacity(this->windowOpacity() - parent->maxOpacity/10);
	fadeTimer->start(FADE_TIMER_TIMEOUT);
	}
	else
	{
	this->setVisible(false);
	}
}

///////////////////////////////////////////////////////////////////////////////////////


void NotifyWidget::appendMsg()
{
if(parent->debugMode) fprintf(stderr,"Appending messages\n");
for(std::vector<Message>::iterator iter=messageStack->begin(); iter != messageStack->end(); iter++)	//Check if need to append
	{
	for(std::vector<Message>::iterator iter2=iter; iter2 != messageStack->end(); iter2++)
	{
	if(iter->app_name == iter2->app_name && 
	   iter->header == iter2->header && 
	   !iter2->isComplete  && 
	   (iter->hints["x-canonical-append"].toString() == "allow" || iter->hints["append"].toString() == "allow") && 
	   iter != iter2)
		{
		if(iter2->text.size()>0)iter->text += QString("<br>") + iter2->text;
		iter->isComplete=false;
		iter2->isComplete=true;
		}
	}
	}
for(std::vector<Message>::iterator iter=messageStack->begin(); iter != messageStack->end(); )	//Erase messages, that are already appended
	{
	if(iter->isComplete && iter != messageStack->begin())
		{
		iter = messageStack->erase(iter);
		}
		else
		{
		iter++;
		}
	}
if(parent->debugMode) fprintf(stderr,"Appending complete\n");
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NotifyWidget::hideWidget()
{
if(parent->debugMode) fprintf(stderr,"Attempt to hide widget\n");
this->timer->stop();
emit NotificationClosed(messageStack->front().id,4);
messageStack->erase(messageStack->begin());
if(parent->debugMode) fprintf(stderr,"Erasing first notification is stack\n");
if(messageStack->size()>0)
	{
	this->showWidget();
	}
	else
	{
	if(parent->debugMode) fprintf(stderr,"Starting fade.\n");
	fadeTimer->start(FADE_TIMER_TIMEOUT);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
void NotifyWidget::showWidget()
{
if(parent->debugMode) fprintf(stderr,"Attempt to show new notification\n");
QString myText="";
if(messageStack->size()>0)
{
	this->setStyleSheet(parent->widgetStyle);
	this->setWindowOpacity(parent->maxOpacity);


	// Set icon
	if(!messageStack->front().icon->isNull())
		{
		icon->setPixmap(*messageStack->front().icon);
		}
		else
		{
		icon->setPixmap(QPixmap());
		}
	
	icon->adjustSize();
	//-----------------------------------------------------------------------------
	myText="<html><table border=0><tr>"; //Create table on label
	myText+="<td VALIGN=middle><div>";
	myText+=parent->UrgencyTag[messageStack->front().urgency];
	myText+=messageStack->front().header + QString("</div>");
	if(messageStack->front().text.size()>0)
		{
		myText += messageStack->front().text;
		}
	
	myText+="</td></tr></table>"; // End of table
	//-----------------------------------------------------------------------------

	text->setText(myText);	// Move items
	text->adjustSize();
	if(text->height() < icon->height()) text->resize(text->width(),icon->height());
	if(text->width() < 190) text->resize(190,text->height());
	if(text->height() < 60) text->resize(text->width(),60);


	icon->move(10, 5);
	text->move(icon->width()+icon->pos().x()+10,5);

	this->resize(this->childrenRect().size().width()+10,this->childrenRect().size().height()+10);

	QPixmap pixmap(this->size()); // Cutting QWidget corners
	pixmap.fill(Qt::transparent);
	this->render(&pixmap,QPoint(),QRegion(), DrawChildren | IgnoreMask);
	this->setMask(pixmap.mask());
	
	QPoint pos = parent->getWidgetPosition(this->widgetName);
	this->move(pos.x(), pos.y());


	messageStack->front().isComplete=true;

	this->setVisible(false);
	fadeTimer->stop();
	this->setVisible(true);
	this->timer->start(messageStack->front().timeout);
}			

}
/////////////////////////////////////////////////////////////////////////////////////
void NotifyWidget::checkIfNeedToShow()
{
if(parent->debugMode) fprintf(stderr,"Checking if need to show new notification\n");
if( messageStack->size() > 1 ) this->appendMsg();
if( !this->timer->isActive() || !messageStack->front().isComplete ) this->showWidget();
}
//////////////////////////////////////////////



