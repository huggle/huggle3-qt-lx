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
    def __init__(self, page_name, site, contents):
        self.PageName = page_name
        self.Site = site
        self.Contents = contents

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

# Import all these classes into huggle namespace
huggle.Version = Version
huggle.WikiPage = WikiPage
huggle.WikiEdit = WikiEdit
huggle.WikiSite = WikiSite

# Remove the local names
del locals()['Version']
del locals()['WikiPage']
del locals()['WikiUser']
del locals()['WikiSite']




