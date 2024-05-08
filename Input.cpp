#include "Input.h"

Input::Input(Data *myData, QWidget *parent) : QWidget(parent), data(myData)
{
   // Geometry
   setMinimumSize(760, 60);
   setMaximumSize(760, 60);
   lineEdit->setFixedSize(750, 50);
   lineEdit->move(5, 5);
   instBtn->setFixedSize(150, 40);
   instBtn->move(10, 10);
   instBtn->setIconSize(QSize(20, 20));
   pinBtn->setFixedSize(20, 20);
   pinBtn->move(725, 20);
   pinBtn->setIcon(QIcon(":/Src/AppIco/PushPin_gray.svg"));
   pinBtn->setIconSize(QSize(20, 20));

   // Style
   lineEdit->setFont(QFont("Microsoft YaHei", 14));
   lineEdit->setStyleSheet(
            "border-radius:3px;"
            "padding-left: 155px;"
            "padding-right:35px;"
            "padding-top:1px;"
            "color: black;"
            "background: #FFFFFF;"
            "selection-color: white;"
            "selection-background-color: rgb(143, 143, 143);");
   lineEditShadowEffect->setBlurRadius(5);
   lineEditShadowEffect->setColor(QColor(0, 0, 0, 100));
   lineEditShadowEffect->setOffset(0, 0);
   lineEdit->setGraphicsEffect(lineEditShadowEffect);

   pinBtn->setStyleSheet("background-color: transparent;");

   instBtn->setFont(QFont("Microsoft YaHei", 14, QFont::Bold));
   instBtn->setStyleSheet(
            "QPushButton{"
            "color:black;"
            "background-color: transparent;"
            "}"
            "QPushButton:hover{"
            "padding-top:-5px"
            "}");

   // Component
   renew();
   instBtn->setFocusPolicy(Qt::NoFocus);

   // Signals and events
   connect(this, &Input::upOrDownPressed, this, &Input::loadHistory);
   connect(lineEdit, &QLineEdit::textChanged, this, &Input::grabInstFromLineEdit);
   connect(lineEdit, &QLineEdit::textChanged, [this](){ emit inputChanged(currentInst, lineEdit->text(), lineEdit->cursorPosition()); });
   lineEdit->installEventFilter(this);
}

void Input::renew()
{
    // instBtn
    setInst((data->repeatedInst() != nullptr && data->settings->loadTheReusedInst ? data->repeatedInst() :data->defaultInst), AnimationPath::FromLeft);
    // Welcome
    lineEdit->setPlaceholderText(data->settings->welcomeMsg);
    if(data->settings->getSelectedTextAsDefaultKeywords){
        lineEdit->setText(data->settings->selectedText);
    }
    // Fresh
    justStarted = true;
}

void Input::setInst(Inst *inst, AnimationPath animationPath)
{
   currentInst = inst;
   if (currentInst == nullptr)
   {
      setInstBtnVisible(false);
      instBtn->setIcon(QIcon());
      instBtn->setText("");
   }
   else
   {
      setInstBtnVisible(true);
      instBtn->setIcon(*(currentInst->icon));
      instBtn->setText(shortenInstBtnText(currentInst->name + QString(data->settings->separator).replace("：", ":").replace(" ", "")));
      startInstBtnEnterAnimation(animationPath);
   }
   emit instChanged(currentInst, lineEdit->text(), lineEdit->cursorPosition());
}

void Input::simpifyText()
{
    QString keywords = lineEdit->text();
    int cursorPosition = lineEdit->cursorPosition();

    //Delete inst name in lineEdit
    //   the whole input is contained in the new inst
    if(currentInst->containInNames(keywords, Qt::CaseInsensitive)){
        lineEdit->setText("");
        //   the text near the cursor matched?
    }else if(data->settings->trimKeywordsBeforeCursor){
        // k:the text length before cursor
        for( int k=cursorPosition ; k>=1; --k)
        {
            if(currentInst->containInNames(keywords.mid(cursorPosition-k,k), Qt::CaseInsensitive))
            {
                lineEdit->setText(keywords.remove(cursorPosition-k,k));
                lineEdit->setCursorPosition(cursorPosition-k);
                break;
            }
        }
    }
}

bool Input::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == lineEdit && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Backspace && 0 == lineEdit->cursorPosition() && lineEdit->selectedText().isEmpty())
        {
            setInst(nullptr);
        }
        else if (keyEvent->key() == Qt::Key_Tab)
        {
            lineEdit->setFocus();
        }
        else if (keyEvent->key() == Qt::Key_Escape)
        {
            emit escPressed();
        }
        else if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return)
        {
            emit enterPressed();
        }
        else if (keyEvent->key() == Qt::Key_Up)
        {
            emit upOrDownPressed(Qt::Key_Up);
        }
        else if (keyEvent->key() == Qt::Key_Down)
        {
            emit upOrDownPressed(Qt::Key_Down);
        }
    }
    return QWidget::eventFilter(obj, event);
}

void Input::loadHistory(int keyValue)
{
    //Load history
    if(justStarted && keyValue == Qt::Key_Up){
        setInst(data->instWithName(data->settings->lastInst, Qt::CaseInsensitive), AnimationPath::FromTop);
        lineEdit->setText(data->settings->lastKeyword);

        justStarted = false;
    }
}

void Input::grabInstFromLineEdit()
{
    // Not fresh
    justStarted = false;

    // Website
    if (lineEdit->text().startsWith("http:") || lineEdit->text().startsWith("https:") || lineEdit->text().startsWith("www."))
    {
        setInst(nullptr);
        return;
    }
    // Separator contained
    if (lineEdit->text().contains(data->settings->separator))
    {
        // Capture instructions at the beginning
        int separatorPos = lineEdit->text().indexOf(data->settings->separator);
        QString textBeforeSeparator = lineEdit->text().left(separatorPos);
        Inst* matchedInst = data->instWithName(textBeforeSeparator, Qt::CaseInsensitive);
        if (matchedInst != nullptr)
        { // Matched
            setInst(matchedInst, AnimationPath::FromRight);
            lineEdit->setText( // Warnning: calling the signal bound to itself.
                lineEdit->text().right(
                    lineEdit->text().count() - textBeforeSeparator.count() - data->settings->separator.length())
                );
        }
    }
    // Keyword without a inst
    if (currentInst == nullptr && !lineEdit->text().isEmpty())
    {
        setInst(data->defaultInst, AnimationPath::FromLeft);
    }
}

void Input::startInstBtnEnterAnimation(AnimationPath animationPath)
{
    QPropertyAnimation *animation = new QPropertyAnimation(instBtn, "geometry");

    QRect leftStartRect(-30, 10, 150, 40);
    QRect rightStartRect(40, 10, 150, 40);
    QRect belowStartRect(10, 30, 150, 40);
    QRect topStartRect(10, -10, 150, 40);
    QRect endRect(10, 10, 150, 40);

    if (animationPath == AnimationPath::FromRight)
    {
        animation->setStartValue(rightStartRect);
    }
    else if (animationPath == AnimationPath::FromBelow)
    {
        animation->setStartValue(belowStartRect);
    }
    else if (animationPath == AnimationPath::FromLeft)
    {
        animation->setStartValue(leftStartRect);
    }
    else if (animationPath == AnimationPath::FromTop)
    {
        animation->setStartValue(topStartRect);
    }

    animation->setEndValue(endRect);
    animation->setDuration(200);
    animation->setEasingCurve(QEasingCurve::InOutQuad);

    animation->start();
}

void Input::setInstBtnVisible(bool visible)
{
    // instBtn visibility
    instBtn->setVisible(visible);
    // lineEdit visibility
    if (visible){ lineEdit->setStyleSheet(lineEdit->styleSheet().replace("padding-left: 10px;", "padding-left: 155px;")); }
    else{ lineEdit->setStyleSheet(lineEdit->styleSheet().replace("padding-left: 155px;", "padding-left: 10px;")); }
}

QString Input::shortenInstBtnText(QString text)
{
   QString instName = text.left(text.length() - data->settings->separator.length());
   QFontMetrics fontMetrics(instBtn->font());
   int textReginWidth = instBtn->width() - instBtn->iconSize().width() - 2.5 * 2; // margin

   while (fontMetrics.horizontalAdvance(text) > textReginWidth)
   {
      instName = instName.left(instName.length() - 1);
      text = instName + ".." + data->settings->separator;
   }
   return text;
}



ShadowBtn::ShadowBtn(QWidget *parent) : QPushButton(parent)
{
    //Component
    setFocusPolicy(Qt::NoFocus);

   // style
   shadowEffect->setBlurRadius(0);
   shadowEffect->setColor(QColor(0, 0, 0, 100));
   shadowEffect->setOffset(0, 0);
   setGraphicsEffect(shadowEffect);
}

void ShadowBtn::enterEvent(QEvent *event)
{
   shadowEffect->setBlurRadius(5);
   QPushButton::enterEvent(event);
}

void ShadowBtn::leaveEvent(QEvent *event)
{
   shadowEffect->setBlurRadius(0);
   QPushButton::leaveEvent(event);
}
