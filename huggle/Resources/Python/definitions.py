# this file contains class and constant definitions for huggle
# it is prepended into every single huggle extension
import sys
import huggle

huggle.UNKNOWN_REVID = -1

class Version:
    major = 0
    minor = 0
    revision = 0

class WikiPage:
    PageName = None
    Site = None
    Contents = None

class WikiEdit:
    RevID = huggle.UNKNOWN_REVID
    Page = None
    User = None
    Site = None

class WikiUser:
    Username = None
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

class Marshalling:
    @staticmethod
    def mWikiEdit(edit):
        e_ = huggle.WikiEdit()
        e_.RevID = edit["revid"]
        e_.Page = huggle.Marshalling.mWikiPage(edit["page"])
        e_.User = huggle.Marshalling.mWikiUser(edit["user"])
        #e_.Site = huggle.Marshalling.mWikiSite(edit["site"])
        return e_;

    @staticmethod
    def mWikiPage(page):
        p_ = huggle.WikiPage()
        p_.PageName = page["name"]
        return p_;

    @staticmethod
    def mWikiSite(site):
        s_ = huggle.WikiSite()
        return s_;

    @staticmethod
    def mWikiUser(user):
        u_ = huggle.WikiUser()
        return u_;

huggle.Marshalling = Marshalling

# Remove the local names
del locals()['Version']
del locals()['WikiPage']
del locals()['WikiUser']
del locals()['WikiSite']
del locals()['Marshalling']




