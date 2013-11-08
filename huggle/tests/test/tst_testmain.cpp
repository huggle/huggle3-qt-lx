#include <QString>
#include <QtTest>
#include "../../wikiuser.hpp"

class HuggleTest : public QObject
{
    Q_OBJECT

public:
    HuggleTest();

private Q_SLOTS:
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
