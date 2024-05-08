#include "Data.h"

Data::Data(QObject *parent) : QObject(parent)
{
    refresh(true);
}

Inst *Data::repeatedInst()
{
    if(latestInstNames[0] == latestInstNames[1] && latestInstNames[0] != ""){
        return instWithName(latestInstNames[0]);
    }else{
        return nullptr;
    }
}

int Data::maxRunCnt()
{
    int maxRunCnt = 0;
    for(int i=0; i<=insts.count()-1; ++i){
        if(instAt(i)->runCnt > maxRunCnt){
            maxRunCnt = instAt(i)->runCnt;
        }
    }
    return maxRunCnt;
}

bool Data::itemsRegenerationRequested()
{
#ifdef RELATIVE_PATH
    QFile file(QCoreApplication::applicationDirPath().replace("/", "\\") + "\\RefreshRequired.txt");
#else
    QFile file("E:/Workstation/QtProject/CCSearch/资源文件/RefreshRequired.txt");
#endif
    return file.exists();
}

Inst *Data::instAt(int index)
{
    return &insts[index];
}

Inst *Data::instWithName(QString name, Qt::CaseSensitivity caseSensitivity)
{
    Inst *inst;
    for (inst = insts.begin(); inst != insts.end(); ++inst)
    {
        if ( inst->includeInNames(name,caseSensitivity) )
        {
            return inst;
        }
    }
    return nullptr;
}

void Data::refresh(bool regenerationRequested)
{
    refreshSettings();
    defaultInst = instWithName(settings->defaultInst);
    if(regenerationRequested){
        refreshInsts();
    }else{
        refreshHistoricalInsts();
    }
}

void Data::refreshInsts()
{
    clearInsts();

    // read file
#ifdef RELATIVE_PATH
    QFile file(QCoreApplication::applicationDirPath().replace("/", "\\") + "\\Insts.txt");
#else
    QFile file("E:/Workstation/QtProject/CCSearch/资源文件/Insts.txt");
#endif
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return ;
    }
    QString text = QString::fromUtf8(file.readAll());
    file.close();
    QStringList lines = text.split("\n");

    // Generate insts
    for (int i = 0; i <= lines.length() - 1; ++i)
    {
        qDebug()<< i <<": "<<lines[i];
        Inst inst;
        QStringList instParams = lines[i].split("|");
        if (instParams.count() != 8){ continue; }
        for (int j = 0; j <= instParams.count() - 1; ++j)
        {
            instParams[j].replace(BreakSym, "|").replace(ReturnSym, "\n");
        }
        // name
        inst.name = instParams[0];
        // equivalentNames
        inst.equivalentNames = instParams[1].split(" ", Qt::SkipEmptyParts);
        // fixed?
        inst.isFixed = "true" == instParams[3];
        // operations: the assignment of the icon is based on it, so it must be assigned in advance.
        inst.operations = instParams[5].trimmed().split("\n",Qt::SkipEmptyParts);
        inst.alternateOperationsEnable = instParams[6] == "true";
        inst.alternateOperations = instParams[7].trimmed().split("\n",Qt::SkipEmptyParts);
        // icon
        inst.assignIcon(instParams[2], &settings->defaultBrowser.icon);
        // runCnt
        inst.runCnt = instParams[4].toUInt();

        insts.append(inst);
    }

    // Information extraction for Data
    //   Default inst
    if(instWithName(settings->defaultInst, Qt::CaseInsensitive)!=nullptr){
        defaultInst = instWithName(settings->defaultInst);
    }else if(instWithName("百度")!=nullptr){
        defaultInst = instWithName("百度");
    }else if(instWithName("必应")!=nullptr){
        defaultInst = instWithName("必应");
    }else if(instWithName("谷歌")!=nullptr){
        defaultInst = instWithName("谷歌");
    }
    //   Btn group insts
    for(Inst * inst = insts.begin(); inst!= insts.end();++inst){
        if(inst->isFixed){
            fixedInsts.append(inst);
        }
    }
    refreshHistoricalInsts();
}

void Data::clearInsts()
{
    for(int i= 0; i<=insts.count()-1; ++i){
        //The default browser icon is released by the class Settings itself.
        if(instAt(i)->icon != &settings->defaultBrowser.icon){
            delete instAt(i)->icon;
            instAt(i)->icon = nullptr;
        }
    }
    insts.clear();

    fixedInsts.clear();
    historicalInsts.clear();

    defaultInst = nullptr;
}

void Data::refreshHistoricalInsts()
{
    //Clear
    historicalInsts.clear();

    // read file
#ifdef RELATIVE_PATH
    QFile file(QCoreApplication::applicationDirPath().replace("/", "\\") + "\\Histories.txt");
#else
    QFile file("E:/Workstation/QtProject/CCSearch/资源文件/Histories.txt");
#endif
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){ return; }
    QString text = QString::fromUtf8(file.readAll());
    file.close();
    QStringList lines = text.split("\n");

    for (int i = 0; i <= lines.count() - 1; ++i)
    {
        //In Histories.txt, the latest insts have been placed in the front.
        Inst *inst = instWithName(lines[i], Qt::CaseInsensitive);
        if(nullptr == inst || historicalInsts.contains(inst)){ continue; }
        historicalInsts.append(inst);
    }
}

void Data::refreshSettings()
{
#ifdef RELATIVE_PATH
    QFile file(QCoreApplication::applicationDirPath().replace("/", "\\") + "\\Settings.txt");
#else
    QFile file("E:/Workstation/QtProject/CCSearch/资源文件/Settings.txt");
#endif
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){ return; }
    QString text = QString::fromUtf8(file.readAll());
    file.close();
    QStringList lines = text.split("\n");

    settings->CCSearchActionId = lines[0];
    settings->isQuickerPro = lines[1] == "true";
    settings->defaultInst = lines[2];
    settings->separator = lines[3];
    settings->welcomeMsg = lines[4];
    settings->defaultBrowser.name = lines[5];
    settings->defaultBrowser.dir = lines[6];
    settings->defaultBrowser.assignIcon();
    settings->lastInst = lines[7];
    settings->lastKeyword = lines[8];
    settings->trimKeywordsBeforeCursor = lines[9]== "true";
    settings->getSelectedTextAsDefaultKeywords = lines[10] == "true";
    settings->selectedText = lines[11];
    settings->loadTheReusedInst = lines[12] == "true";
}



Inst::Inst()
{
    hostSrcMap = {
        {"1688.com", ":/Src/BrandIco/1688.svg"},
        {"2.taobao.com", ":/Src/BrandIco/XianYu.svg"},
        {"51job.com", ":/Src/BrandIco/51job.svg"},
        {"58.com", ":/Src/BrandIco/58TongCheng.svg"},
        {"7k7k.com", ":/Src/BrandIco/7K7K.svg"},
        {"91q.com", ":/Src/BrandIco/QianQian.svg"},
        {"acfun.cn", ":/Src/BrandIco/AcFun.svg"},
        {"ai.taobao.com", ":/Src/BrandIco/AiTaoBao.svg"},
        {"aiqicha.baidu.com", ":/Src/BrandIco/AiQiCha.svg"},
        {"amap.com", ":/Src/BrandIco/amap.svg"},
        {"amazon.cn", ":/Src/BrandIco/Amazon.svg"},
        {"amazon.com", ":/Src/BrandIco/Amazon.svg"},
        {"anjuke.com", ":/Src/BrandIco/Anjuke.svg"},
        {"baidu.com", ":/Src/BrandIco/Baidu.svg"},
        {"baike.baidu.com", ":/Src/BrandIco/BaiduEncyclopedia.svg"},
        {"baike.com", ":/Src/BrandIco/TouTiao.svg"},
        {"baixing.com", ":/Src/BrandIco/BaiXing.svg"},
        {"bilibili.com", ":/Src/BrandIco/Bilibili.svg"},
        {"bing.com", ":/Src/BrandIco/Bing.svg"},
        {"book.qq.com", ":/Src/BrandIco/QQRead.svg"},
        {"cankaoxiaoxi.com", ":/Src/BrandIco/CanKaoXiaoXi.svg"},
        {"cctv.com", ":/Src/BrandIco/CCTV.svg"},
        {"chaoxing.com", ":/Src/BrandIco/ChaoXing.svg"},
        {"chinanews.com.cn", ":/Src/BrandIco/ChinaNews.svg"},
        {"chunyuyisheng.com", ":/Src/BrandIco/ChunYuYiSheng.svg"},
        {"cnki.net", ":/Src/BrandIco/cnki.svg"},
        {"cqvip.com", ":/Src/BrandIco/WeiPuWang.svg"},
        {"csdn.net", ":/Src/BrandIco/CSDN.svg"},
        {"ctrip.com", ":/Src/BrandIco/XieCheng.svg"},
        {"dangdang.com", ":/Src/BrandIco/DangDang.svg"},
        {"deepl.com", ":/Src/BrandIco/Deepl.svg"},
        {"docin.com", ":/Src/BrandIco/docin.svg"},
        {"doi.org", ":/Src/BrandIco/DOI.svg"},
        {"douyu.com", ":/Src/BrandIco/Douyu.svg"},
        {"dxy.cn", ":/Src/BrandIco/DingXiangYuan.svg"},
        {"fanqienovel.com", ":/Src/BrandIco/FanQieNovel.svg"},
        {"fanyi.baidu.com", ":/Src/BrandIco/BaiduTranslate.svg"},
        {"gaodun.com", ":/Src/BrandIco/GaoDun.svg"},
        {"getquicker.net", ":/Src/BrandIco/Quicker.svg"},
        {"getquicker.net", ":/Src/BrandIco/Quicker.svg"},
        {"ghpym.com", ":/Src/BrandIco/GuoHeBaoKe.svg"},
        {"github.com", ":/Src/BrandIco/Github.svg"},
        {"google.com", ":/Src/BrandIco/Google.svg"},
        {"google.com.hk", ":/Src/BrandIco/Google.svg"},
        {"guancha.cn", ":/Src/BrandIco/GuanCha.svg"},
        {"huaban.com", ":/Src/BrandIco/HuaBanWang.svg"},
        {"hupu.com", ":/Src/BrandIco/Hupu.svg"},
        {"huya.com", ":/Src/BrandIco/HuYa.svg"},
        {"iconfont.cn", ":/Src/BrandIco/iconfont.svg"},
        {"ifeng.com", ":/Src/BrandIco/ifeng.svg"},
        {"imooc.com", ":/Src/BrandIco/MoocWang.svg"},
        {"index.baidu.com", ":/Src/BrandIco/BaiDuZhiShu.svg"},
        {"instagram.com", ":/Src/BrandIco/Ins.svg"},
        {"iqiyi.com", ":/Src/BrandIco/iQiYi.svg"},
        {"jd.com", ":/Src/BrandIco/jd.svg"},
        {"jianshu.com", ":/Src/BrandIco/JianShu.svg"},
        {"ju.taobao.com", ":/Src/BrandIco/JuHuaSuan.svg"},
        {"kaifa.baidu.com", ":/Src/BrandIco/BaiduKaiFaZhe.svg"},
        {"kanzhun.com", ":/Src/BrandIco/KanZhun.svg"},
        {"ke.qq.com", ":/Src/BrandIco/TencentKeTang.svg"},
        {"kugou.com", ":/Src/BrandIco/KuGou.svg"},
        {"kuwo.cn", ":/Src/BrandIco/KuWo.svg"},
        {"le.com", ":/Src/BrandIco/LeShiTV.svg"},
        {"liepin.com", ":/Src/BrandIco/LiePin.svg"},
        {"mail.126.com", ":/Src/BrandIco/126Mail.svg"},
        {"mail.163.com", ":/Src/BrandIco/163Mail.svg"},
        {"mail.qq.com", ":/Src/BrandIco/QQMail.svg"},
        {"map.baidu.com", ":/Src/BrandIco/BaiduMap.svg"},
        {"metaso.cn", ":/Src/BrandIco/Mita.svg"},
        {"mgtv.com", ":/Src/BrandIco/MangGuoTV.svg"},
        {"miguvideo.com", ":/Src/BrandIco/MiGuVide.svg"},
        {"mogujie.com", ":/Src/BrandIco/MoGuJie.svg"},
        {"mooc.cn", ":/Src/BrandIco/Mooc.svg"},
        {"music.163.com", ":/Src/BrandIco/WangYiYunMusic.svg"},
        {"new.qq.com", ":/Src/BrandIco/Tencent.svg"},
        {"notion.so", ":/Src/BrandIco/Notion.svg"},
        {"open.163.com", ":/Src/BrandIco/WangYiGongKaiKe.svg"},
        {"pptv.com", ":/Src/BrandIco/PPTV.svg"},
        {"qichacha.com", ":/Src/BrandIco/QiChaCha.svg"},
        {"qidian.com", ":/Src/BrandIco/QiDian.svg"},
        {"scholar.google.com", ":/Src/BrandIco/GoogleScholar.svg"},
        {"sf-express.com", ":/Src/BrandIco/ShunFeng.svg"},
        {"shixiseng.com", ":/Src/BrandIco/ShiXiSeng.svg"},
        {"sina.com.cn", ":/Src/BrandIco/WeiBo.svg"},
        {"smzdm.com", ":/Src/BrandIco/ShenMeZhiDeMai.svg"},
        {"sogou.com", ":/Src/BrandIco/Sougou.svg"},
        {"soho.com", ":/Src/BrandIco/SohoNews.svg"},
        {"sohu.com", ":/Src/BrandIco/SohoNews.svg"},
        {"sspai.com", ":/Src/BrandIco/ShaoShuPai.svg"},
        {"suning.com", ":/Src/BrandIco/SuNing.svg"},
        {"taobao.com", ":/Src/BrandIco/TaoBao.svg"},
        {"ted.com", ":/Src/BrandIco/TED.svg"},
        {"tianyancha.com", ":/Src/BrandIco/TianYanCha.svg"},
        {"tieba.baidu.com", ":/Src/BrandIco/BaiduTieBa.svg"},
        {"tmall.com", ":/Src/BrandIco/tmall.svg"},
        {"toutiao.com", ":/Src/BrandIco/TouTiao.svg"},
        {"translate.google.com", ":/Src/BrandIco/GoogleTranslate.svg"},
        {"translate.google.com.hk", ":/Src/BrandIco/GoogleTranslate.svg"},
        {"tv.sohu.com", ":/Src/BrandIco/SouHuShiPin.svg"},
        {"v.qq.com", ":/Src/BrandIco/TencentVideo.svg"},
        {"weibo.com", ":/Src/BrandIco/WeiBo.svg"},
        {"wenku.baidu.com", ":/Src/BrandIco/BaiduWenku.svg"},
        {"wikipedia.org", ":/Src/BrandIco/WiKi.svg"},
        {"xdowns.com", ":/Src/BrandIco/LvMeng.svg"},
        {"xiachufang.com", ":/Src/BrandIco/XiaChuFang.svg"},
        {"xiaohongshu.com", ":/Src/BrandIco/Red.svg"},
        {"y.qq.com", ":/Src/BrandIco/QQMusic.svg"},
        {"youku.com", ":/Src/BrandIco/YouKu.svg"},
        {"youtube.com", ":/Src/BrandIco/YouTube.svg"},
        {"zcool.com.cn", ":/Src/BrandIco/ZhanKu.svg"},
        {"zhihu.com", ":/Src/BrandIco/Zhihu.svg"},
        {"zhipin.com", ":/Src/BrandIco/ZhiPin.svg"},
        {"zxxk.com", ":/Src/BrandIco/zxxk.svg"},
        {"chrome.google.com", ":/Src/BrandIco/chromeYingYongShangDian.svg"},
        {"macklin.cn", ":/Src/BrandIco/Macklin.svg"},
        {"4399.com", ":/Src/BrandIco/4399Game.svg"},
        {"sciencedirect.com", ":/Src/BrandIco/ScienceDirect.svg"},
        {"subhdtw.com", ":/Src/BrandIco/SubHub.svg"},
        {"readnovel.com", ":/Src/BrandIco/XiaoShuoYueDuWang.svg"},
        {"qimao.com", ":/Src/BrandIco/7MaoXiaoShuoWang.svg"},
        {"mastergo.com", ":/Src/BrandIco/MasterGo.svg"},
        {"figma.com", ":/Src/BrandIco/Figma.svg"},
        {"chat.openai.com", ":/Src/BrandIco/ChatGPT.svg"},
        {"js.design", ":/Src/BrandIco/JiShiSheJi.svg"},
        {"pan.baidu.com", ":/Src/BrandIco/BaiduWangPan.svg"},
        {"up.woozooo.com", ":/Src/BrandIco/LanZouYun.svg"},
        {"aliyundrive.com", ":/Src/BrandIco/ALiYunPan.svg"},
        {"yuque.com", ":/Src/BrandIco/YuQue.svg"},
        {"cowtransfer.com", ":/Src/BrandIco/NaiNiuKuaiChuan.svg"},
        {"wenshushu.cn", ":/Src/BrandIco/WenShuShu.svg"},
        {"ximalaya.com", ":/Src/BrandIco/XiMaLaYa.svg"},
        {"oschina.net", ":/Src/BrandIco/MaYun.svg"},
        {"xiumi.us", ":/Src/BrandIco/XiuMi.svg"},
        {"12306.cn", ":/Src/BrandIco/12306.svg"},
        {"douban.com", ":/Src/BrandIco/DouBan.svg"},
        {"douyin.com", ":/Src/BrandIco/DouYin.svg"},
        {"twitter.com", ":/Src/BrandIco/Twitter.svg"},
        {"steampowered.com", ":/Src/BrandIco/Steam.svg"},
        {"pan.quark.cn", ":/Src/BrandIco/QuarkWangPan.svg"},
        {"chsi.com.cn", ":/Src/BrandIco/XueXinWang.svg"},
        {"webofscience.com", ":/Src/BrandIco/WebOfScience.svg"},
        {"earth.google.com", ":/Src/BrandIco/GoogleEarth.svg"},
        {"sharepoint.com", ":/Src/BrandIco/OneDrive.svg"},
        {"quora.com", ":/Src/BrandIco/Quora.svg"},
        {"weread.qq.com", ":/Src/BrandIco/WeixinRead.svg"},
        {"photo.baidu.com", ":/Src/BrandIco/YiKeXiangCe.svg"},
        {"52pojie.cn", ":/Src/BrandIco/WuAiPoJie.svg"},
        {"myseller.taobao.com", ":/Src/BrandIco/QianNiu.svg"},
        {"gemini.google.com", ":/Src/BrandIco/Gemini.svg"},
        {"mail.google.com", ":/Src/BrandIco/GoogleMail.svg"},
        {"shop.bositai.net", ":/Src/BrandIco/BoSiTai.svg"},
        };
}

QStringList Inst::names()
{
    return QStringList() << name << equivalentNames;
}

QStringList Inst::lowerCaseNames()
{
    QStringList lowerCaseNames{name.toLower()};
    for (int i = 0; i <= equivalentNames.count() - 1; ++i)
    {
        lowerCaseNames << equivalentNames[i].toLower();
    }
    return lowerCaseNames;
}

QString Inst::firstOperation()
{
    return operations.count() > 0
               ? operations[0]
               : alternateOperations.count() > 0
                     ? alternateOperations[0]
                     : "";
}

OperationType Inst::operationType(QString myOperation)
{
    QString operation = myOperation != "" ? myOperation : firstOperation();
    // Web search
    if ( operation.startsWith("https:") || operation.startsWith("http:") || operation.startsWith("www.") )
    {
        return OperationType::SearchWeb;
    }
    if (operation.contains("|"))
    {
        operation = operation.left(operation.indexOf("|"));
    }
    // Path open
    QRegularExpression regexPath("^(?:[a-zA-Z]:)?(?:[\\\\/][^\\\\/:*?\"<>|\\r\\n]+)+[\\\\/]?$");
    QRegularExpressionMatch matchPath = regexPath.match(operation);
    if (matchPath.hasMatch())
    {
        return OperationType::OpenPath;
    }
    // Quicker action
    QRegularExpression regexUUID("[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}");
    QRegularExpressionMatch matchUUID = regexUUID.match(operation);
    if (matchUUID.hasMatch())
    {
        return OperationType::QuickerAction;
    }
    return OperationType::Notdefined;
}

void Inst::assignIcon(QString dir, QIcon *defaultBrowserIcon)
{
    // Icon customized
    if (dir.startsWith("./Icons/"))
    { // dir format: ./Icons/*.png
#ifdef RELATIVE_PATH
        QPixmap customIcon(QCoreApplication::applicationDirPath() + "\\Icons\\" + dir.mid(8));
#else
        QPixmap customIcon("E:/Workstation/QtProject/CCSearch/资源文件/Icons/" + dir.mid(8));
#endif
        if (!customIcon.isNull())
        {
            icon = new QIcon(customIcon);
            return;
        }
    }
    // Default icon
    QString firstOperation = this->firstOperation();
    if (firstOperation.contains("|"))
    {
        firstOperation = firstOperation.left(firstOperation.indexOf("|"));
    }
    OperationType type = this->operationType();
    // Path open
    if (type == OperationType::OpenPath)
    {
        icon = new QIcon(CFuncs::fileIcon(firstOperation));
        return;
    }
    // Quicker action
    if (type == OperationType::QuickerAction)
    {
        icon = new QIcon(":/Src/BrandIco/Quicker.svg");
        return;
    }
    // Web search
    if (type == OperationType::SearchWeb)
    {
        QString host = Inst::getHost(firstOperation);

        for (int i = 1; i <= 3 && host.count(".") >= 1; ++i)
        {
            if (hostSrcMap.contains(host))
            {
                icon = new QIcon(hostSrcMap.value(host));
                return;
            }
            //Get the previous level domain name
            if(host.indexOf(".") + 1 > host.count()-1){ break; }
            host = host.mid(host.indexOf(".") + 1);
        }
        icon = defaultBrowserIcon;
        return;
    }
}

bool Inst::containInNames(QString myName, Qt::CaseSensitivity caseSensitivity)
{
    if(name.contains(myName, caseSensitivity)){
        return true;
    }
    for(int i=0; i<=equivalentNames.count()-1; ++i){
        if(equivalentNames[i].contains(myName,caseSensitivity)){
            return true;
        }
    }
    return false;
}

bool Inst::includeInNames(QString myName, Qt::CaseSensitivity caseSensitivity)
{
    if (caseSensitivity == Qt::CaseSensitive)
    {
        return myName == name || equivalentNames.contains(myName, Qt::CaseSensitive);
    }
    else
    {
        return myName.toLower() == name.toLower() || equivalentNames.contains(myName, Qt::CaseInsensitive);
    }
}

QString Inst::getHost(QString &web)
{
    QUrl url(web);
    QString host = url.host();
    if (host.startsWith("www.") && host.length() > 4)
    {
        host = host.mid(4);
    }
    if (host.startsWith(".") && host.length() > 1)
    {
        host = host.mid(1);
    }
    return host;
}



void DefaultBrowser::assignIcon()
{
    if (browserIconMap.contains(name))
    {
        icon = QIcon(browserIconMap.value(name));
    }
    else
    {
        icon = CFuncs::fileIcon(dir);
    }
}
