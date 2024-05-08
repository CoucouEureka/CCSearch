#ifndef BTNGROUP_H
#define BTNGROUP_H

#include <QWidget>
#include <QPushButton>
#include <QVector>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include "../00CommonCode/commonfuncs.h"
#include "Data.h"

#define validWidthRatio 0.9

class Btn : public QPushButton
{
   Q_OBJECT
public:
   Btn(Inst* inst, int cnt, QWidget *parent = nullptr);
   Inst *inst = nullptr;
signals:
   void btnClicked(Inst *inst);
private:
   QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
};



class BtnGroup : public QWidget
{
   Q_OBJECT
public:
   explicit BtnGroup(Data *data, QWidget *parent = nullptr);
   QVector<Btn*> btns ;
   Data *data=nullptr;
   QHBoxLayout *layout = new QHBoxLayout(this);

   Btn* btnAt(int index);
   bool exists(Inst *inst);
   int validWidth();

   void refresh();
   void clear();//Including the btns and spacers
   void setBtns();
   void setspacers();
   bool addBtn(Btn* btn); //Return value: whether added

   void trigerByIndex(int i);

signals:
   void instTrigered(Inst *inst);
};

#endif
