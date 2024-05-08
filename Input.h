#ifndef INPUT_H
#define INPUT_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include "../00CommonCode/commonfuncs.h"
#include "Data.h"

enum class AnimationPath
{
    FromLeft = 0,
    FromRight = 1,
    FromBelow = 2,
    FromTop = 3
};



class ShadowBtn : public QPushButton
{
   Q_OBJECT
public:
    ShadowBtn(QWidget *parent = nullptr);

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
};



class Input : public QWidget
{
    Q_OBJECT
public:
    Input(Data *data, QWidget *parent = nullptr);

    Data *data = nullptr;
    Inst *currentInst = nullptr;
    QLineEdit *lineEdit = new QLineEdit(this);
    QPushButton *instBtn = new QPushButton(this);
    ShadowBtn *pinBtn = new ShadowBtn(this);
    bool justStarted = true;

    void renew();
    //The inst can only be changed in this function.
    void setInst(Inst *inst = nullptr, AnimationPath animationPath = AnimationPath::FromBelow);
    void simpifyText();

signals:
    void escPressed();
    void enterPressed();
    void upOrDownPressed(Qt::Key);
    void instChanged(Inst* currentInst, QString inputText, int cursorPos);
    void inputChanged(Inst* currentInst, QString inputText, int cursorPos);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QGraphicsDropShadowEffect *lineEditShadowEffect = new QGraphicsDropShadowEffect(this);
    //Bound with the up and down keys through signaland slot mechanism.
    void loadHistory(int keyValue);
    void grabInstFromLineEdit();
    //Only called by setCurrentInst()
    void startInstBtnEnterAnimation(AnimationPath animationPath);
    void setInstBtnVisible(bool visible);
    QString shortenInstBtnText(QString);
};

#endif
