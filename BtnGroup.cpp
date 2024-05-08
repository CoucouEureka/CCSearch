#include "BtnGroup.h"

BtnGroup::BtnGroup(Data *myData, QWidget *parent): QWidget(parent), data(myData)
{
   //Geometry
   setMaximumSize(760,35);
   setMinimumSize(760,35);
   layout->setContentsMargins(5, 0, 5, 0);
   setLayout(layout);

   //Component
   refresh();
}

Btn *BtnGroup::btnAt(int index)
{
    return btns[index];
}

bool BtnGroup::exists(Inst *inst)
{

   for(int i =0;i<=btns.count()-1;++i){
       if(btnAt(i)->inst == inst){
         return true;
      }
   }
   return false;
}

int BtnGroup::validWidth()
{
    int validWidth = 0;
    for (int i = 0; i <= layout->count()-1; ++i) {
        QLayoutItem *item = layout->itemAt(i);
        if (item->spacerItem()) { continue; }
        Btn *button = qobject_cast<Btn*>(item->widget());
        if (button) {
            validWidth += button->width();
        }
    }
    return validWidth;
}

void BtnGroup::refresh()
{
   clear();
   setBtns();
   setspacers();
}

void BtnGroup::clear()
{
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if(item->widget()){
            delete item->widget();
        }if(item->spacerItem()){
            delete item->spacerItem();
        }
        delete item;
    }
    btns.clear();
}

void BtnGroup::setBtns()
{
   QVector<Inst *> insts = data->fixedInsts + data->historicalInsts;

   for(int i=0;i<=insts.count()-1;++i){
      if(exists(insts[i])){ continue; }
      // Generate and add the btn
      Btn *btn = new Btn(insts[i], btns.count()+1, this);
      if(!addBtn(btn))
      {
         delete btn;
         break;
      }
      // Signal and shortcut
      connect(btn, &Btn::btnClicked, this, &BtnGroup::instTrigered);
   }
}

void BtnGroup::setspacers()
{
    //too few btns: align left
    if(validWidth() <= width()*0.7){
        layout->setSpacing(20);
        layout->setAlignment(Qt::AlignLeft);
        return ;
    }

    //Adequate btns: align at both ends
    layout->setSpacing(0);
    //   Inserting spacers frome back eliminates the trouble of container size growth.
    for(int i=btns.count()-1; i>=1;--i){
        layout->insertItem(i, new QSpacerItem(5,25, QSizePolicy::Expanding, QSizePolicy::Fixed));
    }
}

bool BtnGroup::addBtn(Btn* btn)
{
   if(validWidth()+btn->width()+10>width()*validWidthRatio){ return false; }

   btns.append(btn);
   layout->addWidget(btns.last());
   return true;
}

void BtnGroup::trigerByIndex(int i)
{
   if(i > btns.count()-1){ return ; }
   emit instTrigered(btnAt(i)->inst);
}


Btn::Btn(Inst *myInst, int cnt, QWidget *parent): QPushButton(parent) ,inst(myInst)
{
   //Component & Geometry
   setFocusPolicy(Qt::NoFocus);
   QHash<int,QString> indexAndText={
      {1,"¹"},{2,"²"},{3,"³"},{4,"⁴"},{5,"⁵"},{6,"⁶"},{7,"⁷"},{8,"⁸"},{9,"⁹"}
   };
   if(inst->isFixed){
      setIcon(QIcon(":/Src/AppIco/Lock.svg"));
   }
   setIconSize(QSize(14,14));
   setFont(QFont("Microsoft YaHei",10));
   QFontMetrics elideFont(font());
   setText(elideFont.elidedText(indexAndText.value(cnt)+inst->name, Qt::ElideRight, (inst->isFixed?91:110)));//120-14(iconSize)-5*3(padding) or 120-5*2(padding)
   int textWidth = elideFont.horizontalAdvance(text());

   setMaximumSize(textWidth+(inst->isFixed?15+5*3:5*2)+4,25);
   setMinimumSize(textWidth+(inst->isFixed?15+5*3:5*2)+4,25);

   //Style
   setStyleSheet(
            QString("QPushButton{")
            +"border-radius:3px;"
            +"background-color: white;"
            +"}"
            +"QPushButton:hover{"
            +"background-color: #EFEEEE;"
            +"padding-top:-3px;"
            +"}"
            );
   shadowEffect->setBlurRadius(5);
   shadowEffect->setColor(QColor(0, 0, 0, 150));
   shadowEffect->setOffset(0, 0);
   setGraphicsEffect(shadowEffect);

   //Signals and events
   connect(this, &Btn::clicked, [this](){emit btnClicked(inst);});
}
