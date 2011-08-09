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




NotifyWidget::NotifyWidget(const char *name, std::vector<Message*> *msg, NotifyArea *p)
{
strcpy(widgetName,name);
messageStack = msg;
parent = p;

text = new QClickLabel(this);
timer = new QTimer(this);
icon = new QClickLabel(this);
fadeTimer = new QTimer();

icon->setScaledContents(true);

signalMap = new QSignalMapper(this);
signalMap->setMapping(timer, 1);
signalMap->setMapping(text, 2);
signalMap->setMapping(icon, 2);


connect(fadeTimer, SIGNAL(timeout()), this, SLOT(fadeWidget()));

connect(timer, SIGNAL(timeout()), signalMap, SLOT(map()));
connect(text, SIGNAL(clicked()), signalMap,SLOT(map()));
connect(icon, SIGNAL(clicked()), signalMap,SLOT(map()));
connect(signalMap, SIGNAL(mapped(int)), this, SLOT(hideWidget(int)));
connect(signalMap, SIGNAL(mapped(QString)), this, SLOT(emitActionInvoked(QString)));

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
for(std::vector<Message*>::iterator iter=messageStack->begin(); iter != messageStack->end(); iter++)	//Check if need to append
	{
	for(std::vector<Message*>::iterator iter2=iter; iter2 != messageStack->end(); iter2++)
	{
	if((*iter)->app_name == (*iter2)->app_name && 
	   (*iter)->header == (*iter2)->header && 
	   !(*iter2)->isComplete  && 
	   iter != iter2)
		{
		if((*iter2)->text.size()>0)(*iter)->text += QString("<br>") + (*iter2)->text;
		(*iter)->isComplete=false;
		(*iter2)->isComplete=true;
		}
	}
	}
for(std::vector<Message*>::iterator iter=messageStack->begin(); iter != messageStack->end(); )	//Erase messages, that are already appended
	{
	if((*iter)->isComplete && iter != messageStack->begin())
		{
		delete *iter;
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


void NotifyWidget::hideWidget(int reason)
{
if(parent->debugMode) fprintf(stderr,"Attempt to hide widget\n");

this->timer->stop();

if(reason==2) emit ActionInvoked((*messageStack->front()).id,"default");
emit NotificationClosed((*messageStack->front()).id, reason);

if(parent->debugMode) fprintf(stderr,"Erasing first notification is stack\n");

delete messageStack->front();
for(int i=0;i<32;i++)
	{
	if(this->actionButton[i] != NULL)
		{
		delete this->actionButton[i];
		this->actionButton[i] = NULL;
		}
		else
		{
		break;
		}
	}

messageStack->erase(messageStack->begin());
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
	icon->setPixmap(*(*messageStack->front()).icon);

	icon->resize(std::min(parent->maxIconSize,(*messageStack->front()).icon->width()),
				 std::min(parent->maxIconSize,(*messageStack->front()).icon->height()));
	//-----------------------------------------------------------------------------
	myText="<html><table border=0><tr>"; //Create table on label
	myText+="<td VALIGN=middle><div>";
	myText+=parent->UrgencyTag[(*messageStack->front()).urgency];
	myText+=(*messageStack->front()).header + QString("</div>");
	if((*messageStack->front()).text.size()>0)
		{
		myText += (*messageStack->front()).text;
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

	int x=text->pos().x(), y=text->pos().y()+text->height()+5;

	for(int i=0; i < (*messageStack->front()).action.size() && i < 32; i+=2) //Construct action buttons. Each even element is action key, each odd element is action text.
		{
		this->actionButton[i/2] = new QPushButton(this);
		this->actionButton[i/2]->move(x,y);
		this->actionButton[i/2]->setText((*messageStack->front()).action.at(i+1));
		this->actionButton[i/2]->resize(text->width(),this->actionButton[i/2]->height());
		connect(this->actionButton[i/2], SIGNAL(clicked()), signalMap, SLOT(map()));
		signalMap->setMapping(this->actionButton[i/2], (*messageStack->front()).action.at(i));
		y+=this->actionButton[i/2]->height()+5;
		this->actionButton[i/2]->setVisible(true);
		}

	this->resize(this->childrenRect().size().width()+20,this->childrenRect().size().height()+10);

	QPixmap pixmap(this->size()); // Cutting QWidget corners
	pixmap.fill(Qt::transparent);
	this->render(&pixmap,QPoint(),QRegion(), DrawChildren | IgnoreMask);
	this->setMask(pixmap.mask());
	
	QPoint pos = parent->getWidgetPosition(this->widgetName, this->size());
	x=pos.x()-this->childrenRect().size().width()-10;
	y=pos.y()-this->childrenRect().size().height()-10;
	if(x<0)x=0; if(y<0)y=0;
	this->move(x,y);


	(*messageStack->front()).isComplete=true;

	this->setVisible(false);
	fadeTimer->stop();
	this->setVisible(true);
	this->timer->start((*messageStack->front()).timeout);
}			

}
/////////////////////////////////////////////////////////////////////////////////////
void NotifyWidget::checkIfNeedToShow()
{
if(parent->debugMode) fprintf(stderr,"Checking if need to show new notification\n");
if( messageStack->size() > 1 && 
   ((*messageStack->front()).hints["x-canonical-append"].toString() == "allow" || 
    (*messageStack->front()).hints["append"].toString() == "allow"))
   this->appendMsg();
if( !messageStack->empty() && (!this->timer->isActive() || !(*messageStack->front()).isComplete) ) this->showWidget();
}
//////////////////////////////////////////////


void NotifyWidget::emitActionInvoked(QString actionId)
{
emit ActionInvoked((*messageStack->front()).id, actionId);
this->hideWidget(4);
}
