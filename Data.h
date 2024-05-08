#ifndef DATA_H
#define DATA_H

#include <QUrl>
#include <QFile>
#include <QIcon>
#include <QHash>
#include <QString>
#include "../00CommonCode/commonfuncs.h"

#define RELATIVE_PATH
#define SEE_RUN_CNT false

#define BreakSym "${[breakSym]}$"
#define ReturnSym "${[returnSym]}$"

enum class OperationType
{
    SearchWeb = 0,
    OpenPath = 1,
    QuickerAction = 2,
    Notdefined = 3
};



class DefaultBrowser
{
public:
    QString name, dir;
    QIcon icon;
    void assignIcon();

private:
    QHash<QString, QString> browserIconMap = {
        {"chrome", ":/Src/BrowserIco/chrome.svg"},
        {"msedge", ":/Src/BrowserIco/msedge.svg"},
        {"firefox", ":/Src/BrowserIco/firefox.svg"},
        {"123jiasu", ":/Src/BrowserIco/123jiasu.svg"},
        {"360ChromeX", ":/Src/BrowserIco/360ChromeX.svg"},
        {"360se", ":/Src/BrowserIco/360se.svg"},
        {"brave", ":/Src/BrowserIco/brave.svg"},
        {"ChromeCore", ":/Src/BrowserIco/ChromeCore.svg"},
        {"liebao", ":/Src/BrowserIco/liebao.svg"},
        {"Maxthon", ":/Src/BrowserIco/Maxthon.svg"},
        {"opera", ":/Src/BrowserIco/opera.svg"},
        {"QQBrowser", ":/Src/BrowserIco/QQBrowser.svg"},
        {"Safari", ":/Src/BrowserIco/Safari.svg"},
        {"SLBrowser", ":/Src/BrowserIco/SLBrowser.svg"},
        {"SoGouExplorer", ":/Src/BrowserIco/SoGouExplorer.svg"},
        {"TheWorld", ":/Src/BrowserIco/TheWorld.svg"},
        {"twinkstar", ":/Src/BrowserIco/twinkstar.svg"},
        {"UCBrowser", ":/Src/BrowserIco/UCBrowser.svg"},
        {"vivaldi", ":/Src/BrowserIco/vivaldi.svg"}};
};



class Settings
{
public:
    QString CCSearchActionId;
    bool isQuickerPro, trimKeywordsBeforeCursor, getSelectedTextAsDefaultKeywords, loadTheReusedInst;
    QString defaultInst, separator, welcomeMsg;
    DefaultBrowser defaultBrowser;
    QString lastInst, lastKeyword, selectedText;
};



class Inst
{
public:
    Inst();

    QString name;
    QStringList equivalentNames;
    QIcon *icon;
    bool isFixed = false;
    int runCnt;
    QStringList operations;
    bool alternateOperationsEnable = false;
    QStringList alternateOperations;

    QStringList names();
    QStringList lowerCaseNames();
    //Delimiters may be contained
    QString firstOperation();
    OperationType operationType(QString myOperation = "");

    void assignIcon(QString dir = "", QIcon * defaultBrowserIcon = nullptr);
    //The given name is contained in at least one of the returned Inst's names.
    bool containInNames(QString myName, Qt::CaseSensitivity caseSensitivity = Qt::CaseInsensitive);
    //The given name is equal at least to one of the returned Inst's names.
    bool includeInNames(QString myName, Qt::CaseSensitivity caseSensitivity = Qt::CaseInsensitive);

private:
    QHash<QString, QString> hostSrcMap;
    static QString getHost(QString &web);
};



class Data : public QObject
{
    Q_OBJECT
public:
    explicit Data(QObject *parent = nullptr);

    Settings *settings = new Settings;
    Inst *repeatedInst();
    QVector<QString> latestInstNames = {"",""};
    QVector<Inst> insts;
    QVector<Inst*> fixedInsts, historicalInsts;
    Inst *defaultInst;

    int maxRunCnt();
    bool itemsRegenerationRequested();
    Inst *instAt(int index);
    //The given name is equal to at least one of the returned Inst's names.
    Inst *instWithName(QString name, Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive);

    // Refresh settings and all inst variables
    void refresh(bool regenerationRequired);
    // Refresh insts.
    //    All inst variables: including insts, fixedinsts, historicalInsts, and defaultInst
    void clearInsts();
    //   All inst variables
    void refreshInsts();
    //   Just historical insts
    void refreshHistoricalInsts();

private:

    void refreshSettings();
};

#endif // DATA_H
