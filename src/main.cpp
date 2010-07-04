#ifdef QTOPIA

#include <qtopiaapplication.h>
#include "raptor.h"

QTOPIA_ADD_APPLICATION(QTOPIA_TARGET, RaptorMainWindow)
QTOPIA_MAIN

#else // QTOPIA

#include <QtGui/QApplication>
#include "raptor.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RaptorMainWindow w;
    w.show();
    return a.exec();
}

#endif // QTOPIA
