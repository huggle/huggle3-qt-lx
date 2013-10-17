#include <QCoreApplication>
#include <QString>
#include <QFile>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    if (argc < 2)
    {
        std::cout << "it would be cool if you gave me makefile path" << std::endl;
        return 400;
    }
    QString f(argv[1]);
    QFile *file = new QFile(f);
    if (!file->open(QIODevice::ReadWrite | QIODevice::Text))
    {
        std::cout << "unable to open a file" << std::endl;
        return 400;
    }
    QString value(file->readAll());
    file->close();
    delete file;
    file = new QFile(f);
    if (!file->open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        std::cout << "unable to open a file" << std::endl;
        return 400;
    }
    
    if (value.contains("install:"))
    {
        value = value.mid(0, value.indexOf("install:"));
        value += QString("\n") + QString("install:\n\t ./build/install\nuninstall:\n\t ./build/uninstall\n\nFORCE:\n");
        file->write(value.toUtf8());
        file->close();
    }
    delete file;
    return 0;
}
