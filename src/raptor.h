#ifndef RAPTOR_H
#define RAPTOR_H

#include <QWidget>
#include <QMainWindow>
#include <QMenuBar>
#include <QTabWidget>
#include <QGridLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QListWidget>
#include <QProcess>
#include <QStringList>

#ifdef QTOPIA
#include <QSoftMenuBar>
#include <QtopiaApplication>
#endif

class PackagesTab : public QWidget
{
    Q_OBJECT    
public:
    PackagesTab(QWidget *parent = 0);
    QLineEdit *mask;
    QListWidget *lw;
    QPushButton *bOk;

private:
    QPushButton *bSearch;
    QGridLayout *layout;
};

class SourcesTab : public QWidget
{
    Q_OBJECT    
public:
    SourcesTab(QWidget *parent = 0);
    QTextEdit *text;

private:
    QGridLayout *layout;
    QPushButton *bSave;
    QPushButton *bUpdate;

private slots:
    void saveClicked();
};

class OutputTab : public QWidget
{
    Q_OBJECT    
public:
    OutputTab(QWidget *parent = 0);
    QTextEdit *text;
    QPushButton *bStop;

private:
    QGridLayout *layout;
};

class RaptorMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    RaptorMainWindow(QWidget* parent = 0, Qt::WindowFlags f = 0);

private:
    enum Mode
    {
        ModeDo,
        ModeUpdate,
        ModeList,
        ModeSearch,
        ModeConsole,
        ModeInfo
    };

    QTabWidget *tabWidget;
    PackagesTab *tabPkgs;
    SourcesTab *tabSrcs;
    OutputTab *tabOutp;
    QProcess *aptProc;
    QAction *infoAction;
    Mode mode;
    QString outbuf;
    QStringList instPkgs;
    QString selname;
    QString conscript;

    void runProc(QString cmd);
    void endProc();
    QString getMask();
    QString extractName(QString s);
    void outText(QString s);

private slots:
    void searchClicked(QString s="");
    void okClicked();
    void updateClicked();
    void stopClicked();
    void lwSelectionChanged();
    void showPkgInfo();
    void sTimerEvent();
    void curTabChanged(int n);
    void pReadyRead();
    void pFinished(int exitCode, QProcess::ExitStatus exitStatus);
};

#endif
