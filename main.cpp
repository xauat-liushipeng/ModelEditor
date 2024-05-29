#include "modeleditor.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ModelEditor w;
    w.show();
    return a.exec();
}
