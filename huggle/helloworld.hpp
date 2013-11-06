//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HELLOWORLD_H
#define HELLOWORLD_H

#include <QDeclarativeItem>
// this is a path to huggle 3 source code, keep it if you
// checkout extensions folder to same folder where you
// have huggle3 folder
#include <iextension.h>

class HelloWorld : public QDeclarativeItem, Huggle::iExtension
{
    Q_OBJECT
    Q_DISABLE_COPY(HelloWorld)

public:
    HelloWorld(QDeclarativeItem *parent = 0);
    ~HelloWorld();
};

QML_DECLARE_TYPE(HelloWorld)

#endif // HELLOWORLD_H

