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
#include "../../configuration.hpp"
#include "../../wikiedit.hpp"
#include "../../terminalparser.hpp"
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
        void testCaseTerminalParser();
        void testCaseConfigurationParse_QL();
        void testCaseCoreTrim();
        void testCaseScores();
};

HuggleTest::HuggleTest()
{
}

void HuggleTest::testCaseConfigurationParse_QL()
{
    QString test = "sample-conf:hi\n\nlist1:\n  a,\nb\n";
    QStringList list = Huggle::Configuration::ConfigurationParse_QL("list1", test);
    QVERIFY2(list.count() == 2, "Invalid result for ConfigurationParse_QL, parsed wrong number of lines");
    test = "sample-conf:hi\n\nlist1:\n  a blab ldf xx.;g gfdsg,\nb,\n  c,\n          d d,\n";
    list = Huggle::Configuration::ConfigurationParse_QL("list1", test);
    QVERIFY2(list.count() == 4, "Invalid result for ConfigurationParse_QL, parsed wrong number of lines");
    QVERIFY2(list.at(2) == "c,", "Invalid result for ConfigurationParse_QL, parsed wrong item on position 3");
}

void HuggleTest::testCaseCoreTrim()
{
    Huggle::Core *c = new Huggle::Core();
    QVERIFY2("hello world" == c->Trim("   hello world "), "wrong result for Core::Trim() when parsing words");
    QVERIFY2("hello" == c->Trim("   hello"), "wrong result for Core::Trim() when parsing words");
    QVERIFY2("hello" == c->Trim("                             hello                            "), "wrong result for Core::Trim() when parsing words");
    delete c;
}

void HuggleTest::testCaseScores()
{
    Huggle::Configuration::HuggleConfiguration->LocalConfig_ScoreWords.clear();
    Huggle::Configuration::HuggleConfiguration->LocalConfig_ScoreWords.append(new Huggle::ScoreWord("fuck", 10));
    Huggle::Configuration::HuggleConfiguration->LocalConfig_ScoreWords.append(new Huggle::ScoreWord("fucking", 20));
    Huggle::Configuration::HuggleConfiguration->LocalConfig_ScoreWords.append(new Huggle::ScoreWord("vagina", 50));
    Huggle::Configuration::HuggleConfiguration->Separators << " " << "." << "," << "(" << ")" << ":" << ";" << "!" << "?" << "/";
    Huggle::GC::gc = new Huggle::GC();
    Huggle::WikiEdit *edit = new Huggle::WikiEdit();
    edit->Page = new Huggle::WikiPage("test");
    edit->User = new Huggle::WikiUser("Harry, the vandal");
    edit->DiffText = "fuck this vagina!";
    edit->Score = 0;
    edit->ProcessWords();
    QVERIFY2(edit->Score == 60, QString("Invalid result for score words: " + QString::number(edit->Score)).toUtf8().data());
    QVERIFY2(edit->ScoreWords.contains("vagina"), "Invalid result for score words");
    QVERIFY2(edit->ScoreWords.contains("fuck"), "Invalud result for score words");
    edit->DiffText = "fuc_k vagina!";
    edit->Score = 0;
    edit->ProcessWords();
    QVERIFY2(edit->Score == 50, QString("Invalid result for score words: " + QString::number(edit->Score)).toUtf8().data());
    QVERIFY2(edit->ScoreWords.contains("vagina"), "Invalid result for score words");
    edit->DiffText = "Hey bob, (fuck) there is some vagina.";
    edit->Score = 0;
    edit->ProcessWords();
    QVERIFY2(edit->Score == 60, "Invalid result for score words");
    QVERIFY2(edit->ScoreWords.contains("vagina"), "Invalid result for score words");
    QVERIFY2(edit->ScoreWords.contains("fuck"), "Invalud result for score words");
    edit->DiffText = "Hey bob, (fuck) there is some vagina, let's fuck that vagina.";
    edit->Score = 0;
    edit->ProcessWords();
    QVERIFY2(edit->Score == 60, QString("Invalid result for score words: " + QString::number(edit->Score)).toUtf8().data());
    QVERIFY2(edit->ScoreWords.contains("vagina"), "Invalid result for score words");
    QVERIFY2(edit->ScoreWords.contains("fuck"), "Invalud result for score words");
    edit->DiffText = "Hey bob, there are vaginas over there";
    edit->Score = 0;
    edit->ProcessWords();
    QVERIFY2(edit->Score == 0, QString("Invalid result for score words: " + QString::number(edit->Score)).toUtf8().data());
    edit->DiffText = "Hey bob, mind if I'd be fucking with you?";
    edit->Score = 0;
    edit->ProcessWords();
    QVERIFY2(edit->Score == 20, QString("Invalid result for score words: " + QString::number(edit->Score)).toUtf8().data());
    edit->DiffText = "Hey bob, stop fuckin that vagina!!";
    edit->Score = 0;
    edit->ProcessWords();
    QVERIFY2(edit->Score == 50, QString("Invalid result for score words: " + QString::number(edit->Score)).toUtf8().data());
    edit->SafeDelete();
    delete Huggle::GC::gc;
    Huggle::GC::gc = NULL;
}

void HuggleTest::testCaseWikiUserCheckIP()
{
    QVERIFY2(Huggle::WikiUser("10.0.0.1").IsIP(), "Invalid result for new WikiUser with username of IP, the result of IsIP() was false, but should have been true");
    QVERIFY2(Huggle::WikiUser("150.30.0.56").IsIP(), "Invalid result for new WikiUser with username of IP, the result of IsIP() was false, but should have been true");
    QVERIFY2((Huggle::WikiUser("355.2.0.1").IsIP() == false), "Invalid result for new WikiUser with username of IP, the result of IsIP() was true, but should have been false");
    QVERIFY2((Huggle::WikiUser("Frank").IsIP() == false), "Invalid result for new WikiUser with username of IP, the result of IsIP() was true, but should have been false");
    QVERIFY2((Huggle::WikiUser("Joe").IsIP() == false), "Invalid result for new WikiUser with username of IP, the result of IsIP() was true, but should have been false");
}

void HuggleTest::testCaseTerminalParser()
{
    QStringList list;
    list.append("huggle");
    list.append("-v");
    Huggle::TerminalParser *p = new Huggle::TerminalParser(list.count(), list);
    p->Silent = true;
    QVERIFY2(p->Parse() == false, "Invalid result for terminal parser");
    list.append("-vvvvvvvvvvvvvvvvv");
    delete p;
    p = new Huggle::TerminalParser(list.count(), list);
    p->Silent = true;
    QVERIFY2(p->Parse() == false, "Invalid result for terminal parser");
    list.append("-vvvvvvhvvvvvvv");
    delete p;
    p = new Huggle::TerminalParser(list.count(), list);
    p->Silent = true;
    QVERIFY2(p->Parse() == true, "Invalid result for terminal parser");
    list.clear();
    list.append("huggle");
    list.append("--help");
    delete p;
    p = new Huggle::TerminalParser(list.count(), list);
    p->Silent = true;
    QVERIFY2(p->Parse() == true, "Invalid result for terminal parser");
    list.clear();
    list.append("huggle");
    list.append("--safe");
    delete p;
    p = new Huggle::TerminalParser(list.count(), list);
    p->Silent = true;
    QVERIFY2(p->Parse() == false, "Invalid result for terminal parser");
    list.clear();
    list.append("huggle");
    list.append("--blabla");
    delete p;
    p = new Huggle::TerminalParser(list.count(), list);
    p->Silent = true;
    QVERIFY2(p->Parse() == true, "Invalid result for terminal parser");
    delete p;
}

QTEST_APPLESS_MAIN(HuggleTest)

#include "tst_testmain.moc"
