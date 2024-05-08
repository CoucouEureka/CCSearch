#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QApplication>
#include <QMessageBox>
#include <QDesktopServices>
#include <QLabel>
#include <QUrl>
#include <QShortcut>
#include <QClipboard>
#include "../00CommonCode/commonfuncs.h"
#include "Data.h"
#include "Input.h"
#include "BtnGroup.h"
#include "Candidate.h"

#define MaxSleepingPeriod 120*60*1000 //x min *60*1000
#define HeartBeatInterval 1*60*1000 //x min *60*1000
#define RecommendationCode "79403-9437"

class Timer : public QTimer{
   Q_OBJECT
public:
   explicit Timer(int totalPeriod , int interval, QWidget *parent = nullptr);

   int totalPeriod, cntdownPeriod;

   void renew();
   void check();
};


class Widget : public QWidget
{
   Q_OBJECT
public:
   explicit Widget(QWidget *parent = nullptr);

   bool isPinned=false;
   Data *data = new Data();
   QLabel *backgroundLabel = new QLabel(this);
   Input *input = new Input(data, this);
   BtnGroup *btnGroup = new BtnGroup(data, this);
   Candidate *candidate = new Candidate(data,this);
   QVBoxLayout *layout = new QVBoxLayout(this);
   QDateTime wakeupTime;

   //Life
   void wakeup();
   void sleep();
   void feedback(Inst *inst, QString keywords);
   void execute(Inst *inst, QString keywords);
   bool togglePinned();
   //Keys
   void cancel();
   void enterPressed();
   void sellDialog();
   void uporDownPressed(Qt::Key pressedkey);

protected:
   void changeEvent(QEvent *event) override;

private:
   QVector<QShortcut *> altShortcuts, ctrlShortcuts;
   QShortcut *newInstShortcut = new QShortcut(QKeySequence(Qt::CTRL+Qt::Key_N),this);
   QShortcut *pinShortcut = new QShortcut(QKeySequence(Qt::CTRL+Qt::Key_P),this);
   QShortcut *topShortcut = new QShortcut(QKeySequence(Qt::CTRL+Qt::Key_T),this);
   Timer *heartBeat = new Timer(MaxSleepingPeriod, HeartBeatInterval, this);
   QFileSystemWatcher *alarmListener = new QFileSystemWatcher(this);
   void moveToActiveScreen();
   void newInst();
};

#endif // WIDGET_H
