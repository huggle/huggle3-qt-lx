//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HUGGLEFEED_H
#define HUGGLEFEED_H

#include "exception.h"

class HuggleQueueFilter;
class WikiEdit;
class HuggleFeed
{
public:
    HuggleFeed();
    virtual ~HuggleFeed();
    //! Return true if this feed is operational or not
    virtual bool IsWorking() { return false; }
    //! Restart the feed engine
    virtual bool Restart() {return false;}
    //! Stop the feed engine
    virtual void Stop() {}
    //! Start the feed engine
    virtual bool Start() { return false; }
    //! This is useful to stop parsing edits from irc and like in case that queue is full
    virtual void Pause() {}
    //! Resume edit parsing
    virtual void Resume() {}
    //! Check if feed is containing some edits in buffer
    virtual bool ContainsEdit() { return false; }
    virtual bool IsPaused() { return false; }
    //! Return a last edit from cache or NULL
    virtual WikiEdit *RetrieveEdit() { return NULL; }
    HuggleQueueFilter *Filter;
};

#endif // HUGGLEFEED_H
