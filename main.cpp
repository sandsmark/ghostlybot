#include "mainloop.h"

#include <QCoreApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    MainLoop me;
    if (!me.connectToHost("localhost", 54321)) {
        qWarning() << "Failed to connect!";
        return 1;
    }

    return a.exec();
}
