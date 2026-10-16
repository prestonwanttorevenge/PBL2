#include <QApplication>
#include <QStyleFactory>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "ui/Login.h"
#include "services/Database.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);
    a.setStyle(QStyleFactory::create("Fusion"));
    QFile styleFile("resources/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        a.setStyleSheet(stream.readAll());
        styleFile.close();
        qDebug() << "==> DA NAP STYLE.QSS THANH CONG!";
    } else {
        qDebug() << "==> LOI: Khong mo duoc file QSS tai:" << styleFile.fileName();
    }

    Database db;
    db.loadAllData();

    Login *w = new Login(db);
    w->show();

    int result = a.exec(); 
    db.saveAllData(); 

    return result;
}