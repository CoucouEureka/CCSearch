#pragma once

#include <QListWidget>
#include <QListWidgetItem>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QEvent>
#include "Data.h"

enum class TitleType{
   Matched=0,
   Frequency=1
};

enum class LstItemType{
   ItemWidget=0,
   Title=1
};

enum class Direction{
   Up=0,
   Down=1,
};



class Title: public QWidget{
   Q_OBJECT
public:
   explicit Title(TitleType titleType, QWidget *parent = nullptr);

   TitleType type;
   QLabel *background = new QLabel(this);
   QLabel *title = new QLabel(this);
};



class Tip : public QWidget{
   Q_OBJECT
public:
   explicit Tip(QWidget *parent = nullptr);

//   QLabel *background = new QLabel(this);
   QLabel *tipLabel = new QLabel("",this);
   void renew();
   void update(Inst *inst, QString inputText, Inst *defaultInst);
};



class ItemWidget : public QWidget
{
   Q_OBJECT
public:
   explicit ItemWidget(Inst *myInst, QWidget *parent = nullptr);

   Inst *inst = nullptr;
   QLabel *background = new QLabel(this);
   QString styleSheetBeforeCursorEnter;
   QLabel *icon= new QLabel(this);
   QLabel *name= new QLabel("",this);
   QLabel *operation= new QLabel("",this);
   QLabel *shortcut= new QLabel("",this);

protected:
   void enterEvent(QEvent* event) override;
   void leaveEvent(QEvent* event) override;
   bool eventFilter(QObject *obj, QEvent *event) override;

signals:
   void myItemWidgetTriggered(Inst *inst);
};



class LstItem : public QListWidgetItem{
public:
   explicit LstItem(Inst *myInst, QListWidget *parent = nullptr);
   explicit LstItem(TitleType myTitleType, QListWidget *parent = nullptr);

   int recommendationLevel = 0;
   Inst *inst = nullptr;
   LstItemType type;
   ItemWidget *itemWidget = nullptr;
   Title *title = nullptr;

   bool operator<(const QListWidgetItem &other) const override;
};



class LstWidget : public QListWidget
{
   Q_OBJECT
public:
   LstWidget(Data *data, QWidget *parent = nullptr);

   Data *data = nullptr;
   int matchedItemsCnt = 0;

   //Refresh
   void refresh(bool regenerationRequired = true);
   void generate();
   void clearWidgets();
   void reSort(const QString input = "", int cursorPosition = 0);
   void assignRecommendationLevel(const QString input = "", int cursorPosition = 0);
   void refreshShortcut();
   //Switch between Items
   void switchItem(int keyValue);
   void correctCurrentRow(Direction);
   void flushHighlighting();

   void trigerByIndex(int index);
signals:
   void instTriggered(Inst *inst);
};



class Candidate : public QWidget{
   Q_OBJECT
public:
   Candidate(Data *myData, QWidget *parent = nullptr);

   Data *data;
   LstWidget *lstWidget = new LstWidget(data,this);
   Tip *tip = new Tip(this);

private:
   QVBoxLayout *layout = new QVBoxLayout(this);
   QLabel *background = new QLabel(this);
   QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
};
