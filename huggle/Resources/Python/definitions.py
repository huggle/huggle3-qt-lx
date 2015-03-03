# this file contains class and constant definitions for huggle

UNKNOWN_REVID = -1

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
    RevID = UNKNOWN_REVID
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
