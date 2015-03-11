# this file contains class and constant definitions for huggle
# it is prepended into every single huggle extension
import sys
import huggle

huggle.UNKNOWN_REVID = -1

class HuggleException(Exception):
     def __init__(self, value):
         self.value = value
     def __str__(self):
         return repr(self.value)

class Version:
    major = 0
    minor = 0
    revision = 0
    def __str__(self):
        return str(major) + "." + str(minor) + "." + str(revision)

class WikiPage:
    PageName = None
    Site = None
    Contents = None
    def GetNS():
        raise HuggleException('Not implemented');
    def IsTalk():
        raise HuggleException('Not implemented');

class WikiEdit:
    RevID = huggle.UNKNOWN_REVID
    Page = None
    User = None
    Site = None
    Minor = False
    Bot = False
    NewPage = False
    Summary = None
    SizeIsKnown = None
    CurrentUserWarningLevel = None
    DiffText = None
    TPRevBaseTime = None
    EditMadeByHuggle = False
    TrustworthEdit = None
    OwnEdit = None
    Score = 0
    ScoreWords = None
    Time = None
    Size = None
    def IsRevert():
        return huggle.is_revert(Summary); 

class WikiUser:
    UserName = None
    Site = None

class WikiSite:
    Name        = None
    URL         = None
    LongPath    = 'wiki/'
    ScriptPath  = 'w/'
    IRCChannel  = None
    XmlRcsName  = None
    OAuthURL    = ''
    HANChannel  = ''
    MediaWikiVersion = None

huggle.Version = Version
huggle.WikiPage = WikiPage
huggle.WikiUser = WikiUser
huggle.WikiEdit = WikiEdit
huggle.WikiSite = WikiSite
huggle.HuggleException = HuggleException

class Marshalling:
    @staticmethod
    def mVersion(version):
        v = huggle.Version()
        v.Major = version["major"]
        v.Minor = version["minor"]
        v.Revision = version["revision"]
        return v;

    @staticmethod
    def mWikiEdit(edit):
        e_ = huggle.WikiEdit()
        e_.RevID = edit["revid"]
        e_.Page = huggle.Marshalling.mWikiPage(edit["page"])
        e_.User = huggle.Marshalling.mWikiUser(edit["user"])
        e_.Minor = edit["minor"]
        e_.NewPage = edit["newpage"]
        e_.Bot = edit["bot"]
        e_.Summary = edit["summary"]
        e_.Site = huggle.Marshalling.mWikiSite(edit["site"])
        return e_;

    @staticmethod
    def mWikiPage(page):
        p_ = huggle.WikiPage()
        p_.PageName = page["name"]
        p_.Site = huggle.Marshalling.mWikiSite(page["site"])
        return p_;

    @staticmethod
    def mWikiSite(site):
        s_ = huggle.WikiSite()
        s_.Name = site["name"]
        s_.LongPath = site["lp"]
        s_.ScriptPath = site["sp"]
        s_.IRCChannel = site["irc"]
        s_.XmlRcsName = site["xmlrcsname"]
        s_.OAuthURL = site["oauth_url"]
        s_.HANChannel = site["han"]
        s_.MediaWikiVersion = huggle.Marshalling.mVersion(site["mediawiki_version"])
        return s_;

    @staticmethod
    def mWikiUser(user):
        u_ = huggle.WikiUser()
        u_.Site = huggle.Marshalling.mWikiSite(user["site"])
        u_.UserName = user["name"]
        return u_;

huggle.Marshalling = Marshalling

# Remove the local names
del locals()['Version']
del locals()['WikiPage']
del locals()['HuggleException']
del locals()['WikiUser']
del locals()['WikiSite']
del locals()['Marshalling']




