//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include <QString>
#include <QtTest>
#include "../../wikiuser.hpp"

//! This is a unit test
class HuggleTest : public QObject
{
    Q_OBJECT

    public:
        HuggleTest();

    private Q_SLOTS:
        //! Test if IsIP returns true for users who are IP's
        void testCaseWikiUserCheckIP();
};

HuggleTest::HuggleTest()
{
}

void HuggleTest::testCaseWikiUserCheckIP()
{
    QVERIFY2(Huggle::WikiUser("10.0.0.1").IsIP(), "Invalid result for new WikiUser with username of IP, the result of IsIP() was false, but should have been true");
    QVERIFY2(Huggle::WikiUser("150.30.0.56").IsIP(), "Invalid result for new WikiUser with username of IP, the result of IsIP() was false, but should have been true");
    ///\todo FIX ME
    //QVERIFY2((Huggle::WikiUser("355.2.0.1").IsIP() == false), "Invalid result for new WikiUser with username of IP, the result of IsIP() was true, but should have been false");
}

QTEST_APPLESS_MAIN(HuggleTest)

#include "tst_testmain.moc"
