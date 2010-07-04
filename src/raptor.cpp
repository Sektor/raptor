#include "raptor.h"
//#include <QDebug>
#include <QTextStream>
#include <QFile>
#include <QMessageBox>
#include <QHostInfo>
#include <QScrollBar>
#include <QTimer>

#define APP_TITLE tr("R(apt)or")
#define SOURCES_LIST "/etc/apt/sources.list"

RaptorMainWindow::RaptorMainWindow(QWidget* parent, Qt::WindowFlags f)
    : QMainWindow(parent, f)
{
    setWindowTitle(APP_TITLE);

#ifdef QTOPIA
    QMenu* m = QSoftMenuBar::menuFor(this);
    QSoftMenuBar::setHelpEnabled(this,true);
#else
    QMenu *m = menuBar()->addMenu(tr("&File"));
    resize(480, 640);
#endif
    m->addAction(tr("Quit"), this, SLOT(close()));

    tabWidget = new QTabWidget(this);
    tabPkgs = new PackagesTab(this);
    lwSelectionChanged();
    tabSrcs = new SourcesTab(this);
    tabOutp = new OutputTab(this);
    tabWidget->addTab(tabPkgs, tr("Packages"));
    tabWidget->addTab(tabSrcs, tr("Sources"));
    tabWidget->addTab(tabOutp, tr("Output"));

    setCentralWidget(tabWidget);

    //--

    bool install=false;
    bool remove=false;
    bool update=false;
    QStringList pkgs;
    QStringList cl_args = QCoreApplication::arguments();
    int ac = cl_args.count();
    if (ac>1)
    {
        int tki=1;
        while(true)
        {
            QString a = cl_args[tki];
            if (a=="-i")
            {
                install=true;
                tki+=1;
            }
            else if (a=="-r")
            {
                remove=true;
                tki+=1;
            }
            else if (a=="-u")
            {
                update=true;
                tki+=1;
            }
            else
            {
                pkgs << a;
                tki+=1;
            }

            if (tki>=ac) break;
        }
    }

    QString script1 = "";
    QString script2 = "";
    if (update)
        script1 = "apt-get update";
    if ((install || remove) && (pkgs.count()>0))
    {
        if (install)
            script2 = "apt-get -y install";
        if (remove)
            script2 = "apt-get -y remove";
        foreach (QString pkg, pkgs)
            script2 = script2 + " " + pkg;
    }
    if ((!script1.isEmpty()) || (!script2.isEmpty()))
    {
        tabPkgs->setEnabled(false);
        tabSrcs->setEnabled(false);
        tabWidget->setCurrentWidget(tabOutp);
        if ((!script1.isEmpty()) && (script2.isEmpty()))
            conscript = script1;
        else if ((script1.isEmpty()) && (!script2.isEmpty()))
            conscript = script2;
        else if ((!script1.isEmpty()) && (!script2.isEmpty()))
            conscript = "sh -c \"" + script1 + "; " + script2 + "\"";
        QTimer::singleShot(0, this, SLOT(sTimerEvent()));
    }
}

void RaptorMainWindow::sTimerEvent()
{
    mode = ModeConsole;
    runProc(conscript);
}

void RaptorMainWindow::searchClicked(QString s)
{
    selname = s;
    mode = ModeList;
    //runProc("sh -c \"dpkg --get-selections | grep " + getMask() + "\"");
    runProc("dpkg --get-selections");
}

void RaptorMainWindow::okClicked()
{
    if (tabPkgs->lw->selectedItems().count() > 0)
    {
        mode = ModeDo;
        QString pname = extractName(tabPkgs->lw->selectedItems()[0]->text());
        selname = pname;
        if (tabPkgs->lw->selectedItems()[0]->checkState() == Qt::Checked)
            runProc("apt-get -y remove " + pname);
        else
            runProc("apt-get -y install " + pname);
    }
}

void RaptorMainWindow::lwSelectionChanged()
{
    if (tabPkgs->lw->selectedItems().count() > 0)
    {
        if (tabPkgs->lw->selectedItems()[0]->checkState() == Qt::Checked)
            tabPkgs->bOk->setText(tr("Remove package"));
        else
            tabPkgs->bOk->setText(tr("Install package"));
        tabPkgs->bOk->setEnabled(true);
    }
    else
    {
        tabPkgs->bOk->setText(tr("Select package"));
        tabPkgs->bOk->setEnabled(false);
    }
}

void RaptorMainWindow::updateClicked()
{
    mode = ModeUpdate;
    runProc("apt-get update");
}

void RaptorMainWindow::stopClicked()
{
    aptProc->terminate();
    //aptProc->kill();
}

void RaptorMainWindow::runProc(QString cmd)
{
    outbuf = "";
    aptProc = new QProcess(this);
    aptProc->setProcessChannelMode(QProcess::MergedChannels);
    connect(aptProc, SIGNAL(readyRead()), this, SLOT(pReadyRead()));
    connect(aptProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(pFinished(int, QProcess::ExitStatus)));
    if ((mode == ModeDo) || (mode == ModeUpdate) || (mode == ModeConsole))
        tabOutp->text->clear();
    outText(QHostInfo::localHostName() + ":~# " + cmd + "\n");
    aptProc->start(cmd);
    if(!aptProc->waitForStarted())
    {
        QMessageBox::critical(this, APP_TITLE, tr("Unable to start") + " '" + cmd + "'");
        endProc();
    }
    else
    {
        tabPkgs->setEnabled(false);
        tabSrcs->setEnabled(false);
        tabOutp->bStop->setEnabled(true);
        if ((mode == ModeDo) || (mode == ModeUpdate) || (mode == ModeConsole))
            tabWidget->setCurrentWidget(tabOutp);
    }
}

void RaptorMainWindow::endProc()
{
    outText(QHostInfo::localHostName() + ":~#\n");
    delete aptProc;
}

QString RaptorMainWindow::getMask()
{
    return "\"" + tabPkgs->mask->text() + "\"";
}

QString RaptorMainWindow::extractName(QString s)
{
    int k = s.indexOf(' ');
    if (k<0)
        k = s.indexOf('\t');
    if (k>=0)
        return s.left(k);
    return s;
}

void RaptorMainWindow::outText(QString s)
{
    tabOutp->text->insertPlainText(s);
    QScrollBar *sb = tabOutp->text->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void RaptorMainWindow::pReadyRead()
{
    QString txt = aptProc->readAll();
    if (txt != "")
    {
        if ((mode == ModeDo) || (mode == ModeUpdate) || (mode == ModeConsole))
            outText(txt);
        if ((mode == ModeList) || (mode == ModeSearch))
            outbuf += txt;
    }
}

void RaptorMainWindow::pFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    //Q_UNUSED(exitStatus);

    endProc();

    tabPkgs->setEnabled(true);
    tabSrcs->setEnabled(true);
    tabOutp->bStop->setEnabled(false);
    if (mode == ModeDo)
    {
        tabWidget->setCurrentWidget(tabPkgs);
        searchClicked(selname);
    }
    else if (mode == ModeUpdate)
    {
        tabWidget->setCurrentWidget(tabSrcs);
    }
    else if (mode == ModeList)
    {
        if (exitStatus == QProcess::NormalExit)
        {
            instPkgs.clear();
            QStringList pkgs = outbuf.split("\n");
            foreach (QString pkg, pkgs)
            {
                QString pname = extractName(pkg);
                if (!pname.isEmpty())
                    instPkgs.append(pname);
            }
            mode = ModeSearch;
            runProc("apt-cache search " + getMask());
        }
    }
    else if (mode == ModeSearch)
    {
        if (exitStatus == QProcess::NormalExit)
        {
            tabPkgs->lw->clear();
            QListWidgetItem *selel = NULL;
            QStringList pkgs = outbuf.split("\n");
            foreach (QString pkg, pkgs)
            {
                QString pname = extractName(pkg);
                if (!pname.isEmpty())
                {
                    bool inst = instPkgs.contains(pname);
                    QListWidgetItem *element = new QListWidgetItem(pkg);
                    element->setCheckState(inst ? Qt::Checked : Qt::Unchecked);
                    element->setFlags(element->flags() & ~Qt::ItemIsUserCheckable);
                    if ((!selname.isEmpty()) && (pname==selname))
                        selel = element;
                    tabPkgs->lw->addItem(element);
                }
            }
            if (selel != NULL)
                selel->setSelected(true);
            else
            {
                QScrollBar *vsb = tabPkgs->lw->verticalScrollBar();
                vsb->setValue(vsb->minimum());
                QScrollBar *hsb = tabPkgs->lw->horizontalScrollBar();
                hsb->setValue(hsb->minimum());
            }
        }
    }
    else if (mode == ModeConsole)
    {
        close();
    } //if mode
}

//=============================================================================

PackagesTab::PackagesTab(QWidget *parent)
    : QWidget(parent)
{
    mask = new QLineEdit(this);
    connect(mask, SIGNAL(returnPressed()), parent, SLOT(searchClicked()));

    bSearch = new QPushButton(this);
    bSearch->setText(tr("Search"));
    connect(bSearch, SIGNAL(clicked()), parent, SLOT(searchClicked()));

    lw = new QListWidget(this);
    connect(lw, SIGNAL(itemSelectionChanged()), parent, SLOT(lwSelectionChanged()));

    bOk = new QPushButton(this);
    connect(bOk, SIGNAL(clicked()), parent, SLOT(okClicked()));

    layout = new QGridLayout(this);
    layout->addWidget(mask, 0, 0);
    layout->addWidget(bSearch, 0, 1);
    layout->addWidget(lw, 1, 0, 1, 2);
    layout->addWidget(bOk, 2, 0, 1, 2);
    setLayout(layout);
}

//=============================================================================

SourcesTab::SourcesTab(QWidget *parent)
    : QWidget(parent)
{
    text = new QTextEdit(this);
    text->setWordWrapMode(QTextOption::NoWrap);

    bSave = new QPushButton(this);
    bSave->setText(tr("Save"));
    connect(bSave, SIGNAL(clicked()), this, SLOT(saveClicked()));

    bUpdate = new QPushButton(this);
    bUpdate->setText(tr("apt-get update"));
    connect(bUpdate, SIGNAL(clicked()), parent, SLOT(updateClicked()));

    layout = new QGridLayout(this);
    layout->addWidget(text, 0, 0);
    layout->addWidget(bSave, 1, 0);
    layout->addWidget(bUpdate, 2, 0);
    setLayout(layout);

    QString txt;
    QFile file(SOURCES_LIST);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        txt = stream.readAll();
        file.close();
    }
    else
        txt = tr("Error reading %1").arg(SOURCES_LIST);
    text->setText(txt);
}

void SourcesTab::saveClicked()
{
    QFile file(SOURCES_LIST);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        stream << text->toPlainText();
        file.close();
    }
    else
        QMessageBox::warning(this, APP_TITLE, tr("Error writing %1").arg(SOURCES_LIST));
}

//=============================================================================

OutputTab::OutputTab(QWidget *parent)
    : QWidget(parent)
{
    text = new QTextEdit(this);
    //text->setWordWrapMode(QTextOption::NoWrap);
    text->setReadOnly(true);

    bStop = new QPushButton(this);
    bStop->setEnabled(false);
    bStop->setText(tr("Stop"));
    connect(bStop, SIGNAL(clicked()), parent, SLOT(stopClicked()));

    layout = new QGridLayout(this);
    layout->addWidget(text, 0, 0);
    layout->addWidget(bStop, 1, 0);
    setLayout(layout);
}
