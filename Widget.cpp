#include "Widget.h"

Widget::Widget(QWidget *parent): QWidget(parent)
{
    //Window
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint|Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowState(Qt::WindowActive);

    //Geometry
    setFixedSize(760,input->height()+btnGroup->height()+candidate->height());
    backgroundLabel->move(5,55);
    backgroundLabel->setFixedSize(750,45);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    layout->addWidget(input);
    layout->addWidget(btnGroup);
    layout->addWidget(candidate);

    moveToActiveScreen();

    //Style
    backgroundLabel->setStyleSheet("background-color: rgba(255, 255, 255, 20);");

    //Component
    input->lineEdit->setFocus();

    //Shortcuts
    for (int i = 1; i <= 9; ++i) {
        QKeySequence ctrlKeySequence = QKeySequence(Qt::CTRL + Qt::Key_0 + i);
        QKeySequence altKeySequence = QKeySequence(Qt::ALT + Qt::Key_0 + i);
        ctrlShortcuts.append(new QShortcut(ctrlKeySequence, this));
        altShortcuts.append(new QShortcut(altKeySequence, this));
    }
    for(int i=0;i<=ctrlShortcuts.count()-1;++i){
        connect(ctrlShortcuts[i], &QShortcut::activated, [=]() { btnGroup->trigerByIndex(i); });
    }
    for(int i=0;i<=altShortcuts.count()-1;++i){
        connect(altShortcuts[i], &QShortcut::activated, [=]() {
            candidate->lstWidget->trigerByIndex(i);
            candidate->lstWidget->scrollToTop();
        });
    }
    connect(topShortcut, &QShortcut::activated ,[this](){
        candidate->lstWidget->setCurrentRow(-1);
        candidate->lstWidget->scrollToTop();
    });
    connect(newInstShortcut, &QShortcut::activated ,[this](){
        sleep();
        newInst();
    });
    connect(pinShortcut, &QShortcut::activated ,this,&Widget::togglePinned);

    //Signals and events
    //   Input
    connect(input->pinBtn, &ShadowBtn::clicked, this, &Widget::togglePinned);
    connect(input, &Input::instChanged, [this](Inst* currentInst, QString inputText){
        input->justStarted = false;
        candidate->tip->update(currentInst, inputText, data->defaultInst);
    });
    connect(input, &Input::inputChanged, [this](Inst* currentInst, QString inputText, int cursorPos){
        input->justStarted = false;
        candidate->tip->update(currentInst, inputText, data->defaultInst);
        candidate->lstWidget->reSort(inputText, cursorPos);
    });
    connect(input, &Input::upOrDownPressed, this, &Widget::uporDownPressed);
    connect(input, &Input::enterPressed, this, &Widget::enterPressed);
    connect(input, &Input::escPressed, this, &Widget::cancel);
    //   btnGroup
    //      There are 2 args in the func "setInst".
    //      Although, the 2ed arg of the slot possesses a default value, it's impossible to be connected via the normal way.
    connect(btnGroup, &BtnGroup::instTrigered, input, [this](Inst *inst){ input->setInst(inst); });
    //   candidateWidget
    connect(candidate->lstWidget, &LstWidget::currentRowChanged, [this](){
        input->justStarted = false;
        input->lineEdit->setFocus();
    });
    //      Triggered by enter-pressing, mouse-clicking, and shortcut-activation.
    connect(candidate->lstWidget, &LstWidget::instTriggered, input, [this](Inst *inst){
        input->setInst(inst);
        input->simpifyText();
    });
    //   Life
#ifdef RELATIVE_PATH
    alarmListener->addPath(QCoreApplication::applicationDirPath().replace("/","\\")+"\\alarm.txt");
#else
    alarmListener->addPath("E:/Workstation/QtProject/CCSearch/资源文件/alarm.txt");
#endif
    connect(alarmListener, &QFileSystemWatcher::fileChanged, this, &Widget::wakeup);


}

void Widget::wakeup()
{
    //Life
    heartBeat->renew();

    //Data and components
    data->refresh(data->itemsRegenerationRequested());
    input->renew();
    btnGroup->refresh();
    candidate->lstWidget->refresh(data->itemsRegenerationRequested());
    candidate->tip->renew();

    //Window
    moveToActiveScreen();
    show();
    input->lineEdit->setFocus();
    activateWindow();

    wakeupTime = QDateTime::currentDateTime();
}

void Widget::sleep()
{
    if(wakeupTime.msecsTo(QDateTime::currentDateTime()) < 350){ return ; }

    input->lineEdit->setText("");
    input->setInst(nullptr);

    QCoreApplication::processEvents();
    hide();
}

void Widget::feedback(Inst *inst, QString keywords)
{
#ifdef RELATIVE_PATH
    QFile file(QCoreApplication::applicationDirPath().replace("/","\\")+"\\Feedback.txt");
#else
    QFile file("E:/Workstation/QtProject/CCSearch/资源文件/Feedback.txt");
#endif

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    if(inst != nullptr){
        out << inst->name + "\n";
    }else if(input->currentInst != nullptr){
        out << input->currentInst->name + "\n";
    }else{
        out<<"\n";
    }
    out << keywords;

    file.close();
}

void Widget::execute(Inst *inst, QString keywords)
{
    candidate->lstWidget->setCurrentRow(-1);

    if (keywords.startsWith("http:") || keywords.startsWith("https:") || keywords.startsWith("www."))
    {
        QDesktopServices::openUrl(keywords);
        return;
    }

    inst = nullptr != inst ? inst : data->defaultInst;
    QStringList operations = inst->alternateOperationsEnable && "" == keywords ? inst->alternateOperations : inst->operations;

    for(int i=0; i<=operations.count()-1;++i){
        QString operation = operations[i];
        OperationType operationType = inst->operationType(operation);

        //Web search
        if( operationType == OperationType::SearchWeb){
            QString encodedUrl = operation.replace("%s", QUrl::toPercentEncoding( keywords ));
            QDesktopServices::openUrl(encodedUrl);
        }

        int delimiterPos = operations[i].indexOf("|");
        QString dir, args;
        if(delimiterPos != -1 && delimiterPos != 0){
            dir = operation.left(delimiterPos);
            args = operation.right(operation.length()-delimiterPos-1);
        }else{
            dir = operation;
        }
        //Path open
        if ( operationType == OperationType::OpenPath) {
            CFuncs::commandRun(
                QString("\"%1\" %2")
                    .arg(dir)
                    .arg(args).replace("%s", keywords)
                );
        }
        //Quicker action
        if ( operationType == OperationType::QuickerAction) {
            if(data->settings->isQuickerPro){
                args = args.replace("%s", keywords)
                           .replace("\"","\\\"");
                CFuncs::commandRun(
                    QString("\"C:\\Program Files\\Quicker\\QuickerStarter.exe\" runaction:%1?\"%2\"")
                        .arg(dir)
                        .arg(args)
                    );
            }else{
                sellDialog();
            }
        }
    }

    inst->runCnt += 1;
    data->latestInstNames.remove(0);
    data->latestInstNames.append(inst->name);

    feedback(inst, keywords);
}

bool Widget::togglePinned()
{
    isPinned = !isPinned;

    if(isPinned){ input->pinBtn->setIcon(QIcon(":/Src/AppIco/PushPin_black.svg"));
    }else{ input->pinBtn->setIcon(QIcon(":/Src/AppIco/PushPin_gray.svg")); }

    return isPinned;
}

void Widget::cancel()
{
    if(candidate->lstWidget->currentRow()!=-1){
        candidate->lstWidget->setCurrentRow(-1);
    }else{
        sleep();
    }
}

void Widget::enterPressed()
{
    Inst *selectedInst = input->currentInst;
    QString keywords = input->lineEdit->text();
    LstItem* item = dynamic_cast<LstItem*>(candidate->lstWidget->currentItem());
    //load inst
    if(candidate->lstWidget->currentRow()!=-1 && item->type == LstItemType::ItemWidget)
    {
        candidate->lstWidget->setCurrentRow(-1);
        candidate->lstWidget->scrollToTop();
        emit candidate->lstWidget->instTriggered(item->itemWidget->inst);

    }else{//execute
        sleep();
        execute(selectedInst, keywords);
    }
}

void Widget::sellDialog()
{
    QMessageBox msgBox;
    msgBox.setText("对不起。由于平台限制，从外部启动Quicker动作的功能仅限Quicker专业版用户使用。"
                   "\n\n必须订阅Quicker专业版以后才能正常运行此指令。");

    QPushButton *copyButton = msgBox.addButton(tr("复制推荐码，前去购买"), QMessageBox::ActionRole);
    msgBox.addButton(tr("暂时退出"), QMessageBox::RejectRole);
    msgBox.exec();

    if (msgBox.clickedButton() == copyButton) {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(RecommendationCode);
        QDesktopServices::openUrl(QString("https://getquicker.net/member/buy"));
    }
}

void Widget::uporDownPressed(Qt::Key pressedKey)
{
    candidate->lstWidget->switchItem(pressedKey);
}

void Widget::changeEvent(QEvent *event){

    if (event->type() == QEvent::ActivationChange
        && !isActiveWindow()
        && !isPinned){
        sleep();
    }
    QWidget::changeEvent(event);

}

void Widget::moveToActiveScreen()
{
    QScreen *activeScreen = QGuiApplication::primaryScreen();
    foreach (QScreen *screen, QGuiApplication::screens()){
        if (screen->geometry().contains(QCursor::pos())) {
            activeScreen = screen;
            break;
        }
    }
    int activeScreenWidth = activeScreen->geometry().width(),
        activeScreenHight = activeScreen->geometry().height(),
        activeScreenX = activeScreen->geometry().x(),
        activeScreenY = activeScreen->geometry().y(),
        newX = activeScreenX + (activeScreenWidth - width())/2,
        newY = activeScreenY + (activeScreenHight - height())/2;
    move(newX,newY);
}

void Widget::newInst()
{
    if(data->settings->isQuickerPro){
        CFuncs::commandRun(
            QString("\"C:\\Program Files\\Quicker\\QuickerStarter.exe\" runaction:%1?\"%2\"")
                .arg(data->settings->CCSearchActionId)
                .arg("新建指令")
            );
    }else{
        sellDialog();
        return ;
    }
}



Timer::Timer(int myMaxSleepingPeriod, int myInterval ,QWidget *parent):
    QTimer(parent),
    totalPeriod(myMaxSleepingPeriod)
{
    //Component
    setInterval(myInterval);
    cntdownPeriod = totalPeriod;
    start();
    //Signal
    connect(this, &Timer::timeout, this, &Timer::check);
}

void Timer::renew()
{
    cntdownPeriod = totalPeriod;
}

void Timer::check()
{
    cntdownPeriod -= interval();
    if(cntdownPeriod<=0){
        qApp->exit();
    }
}
