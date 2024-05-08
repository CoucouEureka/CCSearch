#include "Candidate.h"

Candidate::Candidate(Data *myData, QWidget *parent): QWidget(parent), data(myData)
{
   //Geometry
   setFixedSize(760,lstWidget->height()+tip->height()+5*2);//lstItemWidget * 9, tipWidget * 1, blank * 2
   background->setFixedSize(size()-QSize(10,10));
   background->move(5,5);
   layout->setAlignment(Qt::AlignCenter);
   layout->setContentsMargins(0,5,0,5);
   layout->addWidget(lstWidget);
   layout->addWidget(tip);
   layout->setSpacing(0);

   //Style
   background->setStyleSheet("border-radius:3px; background-color:white;");
   background->lower();
   shadowEffect->setBlurRadius(5);
   shadowEffect->setColor(QColor(0, 0, 0, 100));
   shadowEffect->setOffset(0, 0);
   background->setGraphicsEffect(shadowEffect);
}



LstWidget::LstWidget(Data *myData, QWidget *parent) :QListWidget(parent), data(myData)
{

   //Geometry
   setFixedSize(750,60*9+30);//lstItemWidget * 9

   //Component
   refresh(true);

   //Style
   setStyleSheet("QListWidget{"
                 "  border:none;"
                 "  background-color:white;"
                 "  border-top-left-radius:3px;"
                 "  border-top-right-radius:3px;"
                 "}"
                 "QScrollBar:vertical {"
                 "  border: none;"
                 "  border-radius: 3px;"
                 "  background: transparent;"
                 "  width: 4px;"
                 "}"
                 "QScrollBar::handle:vertical {"
                 "  background: lightGray;"
                 "  min-height: 20px;"
                 "  border-radius: 3px;"
                 "}"
                 "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
                 "  height: 0;"
                 "}"
                 );
   setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

   connect(this,&LstWidget::currentRowChanged,this, &LstWidget::flushHighlighting);
}

void LstWidget::refresh(bool regenerationRequired)
{
    if(regenerationRequired){
        clearWidgets();
        clear();
        generate();
    }
    reSort();

    if(!SEE_RUN_CNT){
        refreshShortcut();
    }else{
        for(int i=0; i<=count()-1 ; ++i)
        {
            LstItem* item = dynamic_cast<LstItem*>(this->item(i));
            if(item->type == LstItemType::ItemWidget){
                item->itemWidget->shortcut->setText(QString::number(item->recommendationLevel));
            }
        }
    }
}

void LstWidget::generate()
{
    //Title
    LstItem *matchedTitle = new LstItem(TitleType::Matched,this);
    LstItem *frequencyTitle = new LstItem(TitleType::Frequency,this);
    addItem(matchedTitle);
    setItemWidget(matchedTitle,matchedTitle->title);
    addItem(frequencyTitle);
    setItemWidget(frequencyTitle,frequencyTitle->title);
    //ItemWidget
    for(int i=0;i<=data->insts.count()-1;++i){
        LstItem *item = new LstItem(data->instAt(i), this);
        item->setSizeHint(item->itemWidget->size());
        addItem(item);
        setItemWidget(item, item->itemWidget);
        connect(item->itemWidget, &ItemWidget::myItemWidgetTriggered, [this](Inst *myInst){ emit instTriggered(myInst); });
    }
    //Feedback
#ifdef RELATIVE_PATH
    QFile file(QCoreApplication::applicationDirPath().replace("/", "\\") + "\\RefreshRequired.txt");
#else
    QFile file("E:/Workstation/QtProject/CCSearch/资源文件/RefreshRequired.txt");
#endif
    if (file.exists()) { file.remove(); }
}

void LstWidget::clearWidgets()
{
    for(int i=0 ;i<=count()-1; ++i){
        LstItem* item = dynamic_cast<LstItem*>(this->item(i));
        if(item->itemWidget != nullptr){ delete item->itemWidget; }
        if(item->title != nullptr){ delete item->title; }
    }
}

void LstWidget::reSort(const QString input, int cursorPosition)
{
   //Impossible form
   //   Two signals are sent almost simultaneously from the inputWidget.
   //   The signal that should have arrived early unexpectedly arrived late.
   //   Here, filter must be set.
   if(input.contains(data->settings->separator)){
      int separatorPosition = input.indexOf(data->settings->separator);
      QString textBeforeSeparator = input.left(separatorPosition);
      //matched
      if(nullptr != data->instWithName(textBeforeSeparator)){ return ; }
   }

   //Sort
   assignRecommendationLevel(input, cursorPosition);
   sortItems(Qt::DescendingOrder);
   refreshShortcut();

   //Go back to the top
   setCurrentRow(-1);
   scrollToTop();

}

void LstWidget::assignRecommendationLevel(const QString input, int cursorPosition)
{
    matchedItemsCnt = 0;

    int maxRecommendationLevel = data->maxRunCnt();
    LstItem* matchedTitle = nullptr;
    LstItem* frequencyTitle = nullptr;

    //Cal recommendation level
    for(int i=0; i<=count()-1 ; ++i)
    {
        LstItem* item = dynamic_cast<LstItem*>(this->item(i));
        //rule:
        //    matched tip:                     +maxRecommendationLevel+2
        //    the same:                        +100
        //    input contained:                 +8
        //    inst name contained:             +6
        //    cursor-nearby text contained:    +2
        //    frequency tip:                   +1
        if(LstItemType::ItemWidget == item->type)
        {
            item->recommendationLevel = item->itemWidget->inst->runCnt;
            if(""==input){ continue; }//"" is a substring of all strings.
            QStringList lowerCaseNames = item->itemWidget->inst->lowerCaseNames();
            //The same
            if(lowerCaseNames.contains(input.toLower())){
                item->recommendationLevel += data->maxRunCnt();
                item->recommendationLevel += 100;
            }else{
                for (int j=0;j<=lowerCaseNames.count()-1;++j) {
                    //The whole input is prioritized for matching.
                    if(lowerCaseNames[j].contains(input,Qt::CaseInsensitive)||input.contains(lowerCaseNames[j],Qt::CaseInsensitive)){
                        item->recommendationLevel += data->maxRunCnt();
                        if(lowerCaseNames[j].contains(input,Qt::CaseInsensitive)){ item->recommendationLevel += 8; }
                        if(input.contains(lowerCaseNames[j],Qt::CaseInsensitive)){ item->recommendationLevel += 6; }
                        continue;
                    }
                    //Can the text near the cursor be matched?
                    //   The length of the intercepted text(k):
                    //      <= 4
                    //      <= the length of text before cursor.
                    for(int k=1;k<=4 && k<=cursorPosition;++k){
                        if(lowerCaseNames[j].contains(input.mid(cursorPosition-k,k),Qt::CaseInsensitive)){
                            item->recommendationLevel +=data-> maxRunCnt() + 2;
                        }
                    }
                }
            }

            if(item->recommendationLevel > item->itemWidget->inst->runCnt){//matched?
                ++matchedItemsCnt;
            }
            maxRecommendationLevel = item->recommendationLevel > maxRecommendationLevel ? item->recommendationLevel : maxRecommendationLevel;
        }
        if(LstItemType::Title == item->type ){
            if(TitleType::Frequency == item->title->type){ frequencyTitle = item; }
            if(TitleType::Matched == item->title->type){ matchedTitle = item; }
        }
    }

    if(nullptr!=matchedTitle){
        matchedTitle->recommendationLevel = maxRecommendationLevel+2;
        matchedTitle->setHidden(0==matchedItemsCnt);
    }
    if(nullptr!=frequencyTitle){
        frequencyTitle->recommendationLevel = 1 + data->maxRunCnt();
    }
}

void LstWidget::refreshShortcut()
{
    int shortcutAssignedItemCnt = 0;
    for(int i=0; i<=count()-1 ; ++i){
        LstItem* item = dynamic_cast<LstItem*>(this->item(i));
        if(item->type == LstItemType::ItemWidget){
            if(shortcutAssignedItemCnt < 9){
                item->itemWidget->shortcut->setText(QString("Alt+%1").arg(shortcutAssignedItemCnt+1));
                ++shortcutAssignedItemCnt;
            }else{
                item->itemWidget->shortcut->setText("");
            }
        }
    }
}

void LstWidget::switchItem(int keyValue)
{
   if(keyValue==Qt::Key_Up){
      if(-1==currentRow()){ return; }
      if(0==currentRow()){
         setCurrentRow(-1);
         return;
      }else{
         setCurrentRow(currentRow()-1);
         correctCurrentRow(Direction::Up);
      }
   }else if(keyValue==Qt::Key_Down){
      if(currentRow()==count()-1){ return; }
      if(currentRow()!=count()-1){
         setCurrentRow(currentRow()+1);
         correctCurrentRow(Direction::Down);
      }
   }
}

//Correct errors caused by key pressing
void LstWidget::correctCurrentRow(Direction direction)
{
   if(currentRow()==-1){ return; }
   for(int i=0;i<=2;++i){//Move up to 3 times{
      int inspectedRow = currentRow()+i*(direction==Direction::Down?1:-1);
      if(inspectedRow<-1){
         setCurrentRow(-1);
         return;
      }
      if(inspectedRow>count()-1){
         setCurrentRow(count()-1);
         return;
      }
      LstItem* item = dynamic_cast<LstItem*>(this->item(inspectedRow));
      if(inspectedRow == -1 || item->type!=LstItemType::Title){
         setCurrentRow(inspectedRow);
         return;
      }
   }
}

void LstWidget::flushHighlighting()
{
   for(int i = 0; i<=count()-1;++i ){
      const LstItem* item = dynamic_cast<const LstItem*>(this->item(i));
      if(item->type==LstItemType::ItemWidget){
         item->itemWidget->background->setStyleSheet(
                  QString("border-radius:3px;background-color:%1;")
                  .arg(i==currentRow()?"#DDADD8E6":"white")
                  );
         item->itemWidget->styleSheetBeforeCursorEnter = item->itemWidget->background->styleSheet();
      }
   }
}

void LstWidget::trigerByIndex(int index)
{
   int currentIndex = -1;
   for(int i=0;i<=count()-1;++i){
      LstItem* item = dynamic_cast<LstItem*>(this->item(i));
      if(item->type == LstItemType::ItemWidget){
         ++currentIndex;
         if(currentIndex==index){
            emit instTriggered(item->itemWidget->inst);
         }
      }
   }
}



LstItem::LstItem(Inst *myInst, QListWidget *myLstView)
    :QListWidgetItem(myLstView)
    ,inst(myInst)
{
    itemWidget = new ItemWidget(inst);
    setSizeHint(itemWidget->size());
    type = LstItemType::ItemWidget;
}

LstItem::LstItem(TitleType myTitleType, QListWidget *myLstView)
    :QListWidgetItem(myLstView)
{
    title = new Title(myTitleType);
    setSizeHint(title->size());
    type = LstItemType::Title;
}

bool LstItem::operator<(const QListWidgetItem &other) const
{
    const LstItem* otherItem = dynamic_cast<const LstItem*>(&other);

    if(recommendationLevel == otherItem->recommendationLevel){
        if(
            type == LstItemType::ItemWidget
            &&otherItem->type == LstItemType::ItemWidget
            ){
            return itemWidget->inst->name >= otherItem->itemWidget->inst->name;
        }
    }

    return recommendationLevel < otherItem->recommendationLevel;

}



ItemWidget::ItemWidget(Inst *myInst, QWidget *parent) : QWidget(parent), inst(myInst)
{
   //Geometry
   setFixedSize(750, 60);
   icon->setFixedSize(40,40);
   name->setFixedHeight(32);
   name->setFixedWidth(600);
   operation->setFixedHeight(20);
   operation->setFixedWidth(600);
   shortcut->setFixedSize(70, 30);
   background->setFixedSize(size());

   //Layout
   QVBoxLayout *instNameAndOperationLayout = new QVBoxLayout;
   instNameAndOperationLayout->setContentsMargins(0,5,0,5);
   instNameAndOperationLayout->addWidget(name);
   instNameAndOperationLayout->addWidget(operation);
   instNameAndOperationLayout->setSpacing(6);
   QHBoxLayout *lstViewItemWidgetLayout = new QHBoxLayout;
   lstViewItemWidgetLayout->setContentsMargins(15,0,5,0);
   lstViewItemWidgetLayout->addWidget(icon);
   lstViewItemWidgetLayout->addSpacerItem(new QSpacerItem(0,2,QSizePolicy::Expanding,QSizePolicy::Fixed));
   lstViewItemWidgetLayout->addLayout(instNameAndOperationLayout);
   lstViewItemWidgetLayout->addSpacerItem(new QSpacerItem(0,2,QSizePolicy::Expanding,QSizePolicy::Fixed));
   lstViewItemWidgetLayout->addWidget(shortcut);
   setLayout(lstViewItemWidgetLayout);

   //style
   background->setStyleSheet("border-radius:3px;background-color:white;");
   icon->setStyleSheet("border-radius:5px;");
   name->setFont(QFont("Microsoft YaHei",12,QFont::Normal));
   operation->setFont(QFont("Microsoft YaHei",9,QFont::Normal));
   operation->setStyleSheet("QLabel{color: #3C3C3C;}");
   shortcut->setFont(QFont("Microsoft YaHei",10,QFont::Bold));
   shortcut->setStyleSheet("QLabel{ color: #D9D9D9; }");

   //component
   icon->setPixmap(inst->icon->pixmap(icon->size()));
   QFontMetrics instNameElideFont(name->font());
   name->setText(inst->name);
   name->setText(
            QString("<span style=\"font-size: 12pt; color: black;\">%2</span>").arg(inst->name)
            +(
               inst->equivalentNames.count()>0
               ?QString("<span style=\"font-size: 9.5pt; color: gray;\">%1</span>").arg("/"+inst->equivalentNames.join("/"))
              :""
                )
            );
   QFontMetrics oetailElideFont(operation->font());
   operation->setText(
            QString("<font color=\"#3C3C3C\">%1</font>")
            .arg(oetailElideFont.elidedText( inst->firstOperation(), Qt::ElideRight, operation->maximumWidth()-10))
            .replace("%s", "<font color=\"#1B87D9\">%s</font>")
            );


   installEventFilter(this);
}

void ItemWidget::enterEvent(QEvent *event)
{
   styleSheetBeforeCursorEnter = background->styleSheet();
   background->setStyleSheet("border-radius:3px;background-color:#EFEEEE;");
   QWidget::enterEvent(event);
}

void ItemWidget::leaveEvent(QEvent* event){
   background->setStyleSheet(styleSheetBeforeCursorEnter);
   QWidget::enterEvent(event);
}

bool ItemWidget::eventFilter(QObject *obj, QEvent *event)
{
   if (event->type() == QEvent::MouseButtonPress) {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
      if (mouseEvent->button() == Qt::LeftButton) {
         emit myItemWidgetTriggered(inst);
      } else if (mouseEvent->button() == Qt::RightButton) {
      }
   }
   return QObject::eventFilter(obj, event);
}



Title::Title(TitleType titleType, QWidget *parent): QWidget(parent)
{
   //geometry
   setFixedSize(750,30);
   background->setFixedSize(size());
   background->move(0,0);
   title->setText(titleType == TitleType::Matched?"匹配":"推荐");
   title->move(15,5);

   //style
   background->setStyleSheet(
            "QLabel{"
            "background-color:white;"
            "border-radius:3px;"
            "}"
            );
   title->setFont(QFont("Microsoft YaHei",12,QFont::Bold));
   title->setStyleSheet("color:#B3B3B3;");

   type = titleType;
}



Tip::Tip(QWidget *parent):
   QWidget(parent)
{
   //Geometry
   setFixedSize(750,30);
//   background->setFixedSize(size());
//   background->move(0,0);
   tipLabel->move(15,5);
   tipLabel->setFixedSize(730,20);

   //Component
   renew();

   //Style
//   background->setStyleSheet(
//            "QLabel{"
//            "background-color:white;"
//            "border-bottom-left-radius:3px;"
//            "border-bottom-right-radius:3px;"
//            "}"
//            );
   tipLabel->setFont(QFont("Microsoft YaHei",9,QFont::Normal));
   tipLabel->setStyleSheet("color:#B3B3B3;");
}

void Tip::renew()
{
    tipLabel->setText("↑：恢复上次内容（仅启动）；   Ctrl+[1-9]：从按钮组加载指令；   Ctrl+P：固定/解除固定。");
}

void Tip::update(Inst *inst, QString inputText, Inst *defaultInst)
{
   const Inst *instToExecute = inst!=nullptr ? inst :defaultInst;
   QString tipText;
   QFontMetrics font(tipLabel->font());
   //Operations
   if(!(inputText == "" && instToExecute->alternateOperationsEnable)){
      tipText = QString("将以%1指令，执行关键词：%2").arg( instToExecute->name ).arg(inputText);
   }else{//Alternate operations
      tipText = QString("未输入关键词，将执行%1指令的备用操作列表。").arg( instToExecute->name );
   }
   tipLabel->setText( font.elidedText( tipText , Qt::ElideRight, tipLabel->maximumWidth()) );
}
