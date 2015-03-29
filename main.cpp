#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    a.setStyle(QStyleFactory::create("Fusion"));

#if 0
    QPalette p = qApp->palette();
    p.setColor(QPalette::Window, QColor(53,53,53));
    p.setColor(QPalette::Button, QColor(53,53,53));
    p.setColor(QPalette::Highlight, QColor(142,45,197));
    p.setColor(QPalette::ButtonText, QColor(255,255,255));
    p.setColor(QPalette::WindowText, QColor(255,255,255));

    p.setColor(QPalette::Background, QColor(53,53,53));
    p.setColor(QPalette::Text, QColor(255,255,255));

    a.setPalette(p);
#endif



    QFile file(":/qss/dark.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    a.setStyleSheet(styleSheet);



    return a.exec();
}
