// IMPORTANT: this file has a different license than rest of huggle

//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2014

#ifndef COLLECTABLE_H
#define COLLECTABLE_H

#include "definitions.hpp"

#include <QMutex>
#include <QList>
#include <QString>
#include <QStringList>
#include "gc.hpp"

namespace Huggle
{
    //! Base for all items that are supposed to be collected by garbage collector

    //! In order for any item to be maintained by garbage collector it must be inherited from
    //! this class.
    //! Every object by default when created is unmanaged, which means, that garbage collector doesn't
    //! care about it. In order to change it to managed object, you have to register at least one consumer.
    //! Once you do that, the class must not be explicitly deleted using delete, if you do that
    //! unrecoverable exception will be thrown. The class which is managed (you can verify that by calling Collectable::IsManaged)
    //! can be only deleted by garbage collector when no consumers are using it. Basically every
    //! object that has 0 consumers, will be deleted.

    //! \image html ../documentation/gc01.png
    class HUGGLE_EX Collectable
    {
        public:
            /*!
             * \brief This function is useful when you need to create an extension that uses own collectables in GC
             * \return pointer to last cid
             */
            static unsigned long *GetLastCIDPtr();
#ifdef HUGGLE_PROFILING
            static unsigned long LockCt;
#endif

            Collectable();
            virtual ~Collectable();
            /*!
             * \brief IsManaged Managed class is deleted by GC and must not be deleted by hand
             * \return whether the class is managed
             */
            bool IsManaged();
            //! \brief Tries to delete the object immediately if not used anymore
            //! Use this if you are not sure if you can delete this object in this moment
            //! If the object is not deleted after calling this, it will be managed by GC
            virtual bool SafeDelete();
            //! You can change this to reclaimable by calling this

            //! Reclaimable object can register new consumers even after last consumer was deleted. This shouldn't be ever
            //! used by any other but atomic functions, because in theory collectable which lost its last consumer might
            //! be deleted by GC any time.
            void SetReclaimable();
            //! Whether the object is locked (other threads can't register nor unregister consumers
            //! neither it is possible to delete this object by any other thread)
            bool IsLocked();
            /*!
             * \brief Lock this object so that other threads can't change consumers or modify its properties
             */
            void Lock();
            /*!
             * \brief Unlock this object for deletion by other threads
             */
            void Unlock();
            /*!
             * \brief Registers a consumer
             *
             * This function will store a string which prevent the object from being removed
             * by GC, by calling this function you change type to managed
             * \param consumer String that lock the object
             */
            void RegisterConsumer(int consumer);
            /*!
             * \brief This function will remove a string which prevent the object from being removed
             * \param consumer Unique string that unlock the object
             */
            void UnregisterConsumer(int consumer);
            /*!
             * \brief Registers a consumer
             *
             * This function will store a string which prevent the object from being removed
             * by QueryGC, by calling this function you change the query type to managed
             * \param consumer String that lock the object
             */
            void RegisterConsumer(const QString consumer);
            /*!
             * \brief This function will remove a string which prevent the object from being removed
             * \param consumer Unique string that unlock the object
             */
            void UnregisterConsumer(const QString consumer);
            /*!
             * \brief IncRef This function will add 1 to reference counter and change the collectable to managed in case it wasn't
             *
             * This function is thread safe, it's highly recommended to use Collectable_SmartPtr which
             * will handle the reference counter for you!
             */
            void IncRef();
            /*!
             * \brief DecRef Decrease reference counter
             *
             * If collectable isn't managed or reference counter is 0 it will throw exception, it's highly recommended to use
             * Collectable_SmartPtr which will handle the reference counter for you
             */
            void DecRef();
            /*!
             * \brief DebugHgc
             * \return debug info
             */
            QString DebugHgc();
            /*!
             * \brief CollectableID
             * \return
             */
            unsigned long CollectableID();
            bool HasSomeConsumers();
        private:
            static QString ConsumerIdToString(const int id);
            static QMutex *WideLock;
            static unsigned long LastCID;

            void SetManaged();
            unsigned long CID;
            //! Internal variable that contains a cache whether object is managed
            bool _collectableManaged;
            //! List of string consumers that are using this object

            //! Every consumer needs to use unique string that identifies them
            QStringList Consumers;
            //! Changing to true will prevent an exception from being thrown if you register consumer after deleting last consumer

            //! Doing so may result in unpredictable crashes, because object should never be accessed after last consumer was removed
            bool ReclaimingAllowed;
            //! List of int consumers that are using this object

            //! Every consumer needs to use a unique int that identifies them
            //! if you aren't sure what number to use, or if you are working
            //! in extension you should use string instead
            QList<int> iConsumers;
            QMutex *_collectableQL;
            unsigned int _collectableRefs;
            bool _collectableLocked;
    };

    //_________________________________________________________________________
    // inline defs
    // you will not see any interesting declarations of collectable here
    //_________________________________________________________________________


    inline bool Collectable::HasSomeConsumers()
    {
        return (this->_collectableRefs > 0 || this->iConsumers.count() > 0 || this->Consumers.count() > 0);
    }

    inline void Collectable::IncRef()
    {
        this->_collectableRefs++;
        this->SetManaged();
    }

    inline unsigned long Collectable::CollectableID()
    {
        return this->CID;
    }

    inline bool Collectable::IsLocked()
    {
        return this->_collectableLocked;
    }

    inline unsigned long *Collectable::GetLastCIDPtr()
    {
        return &Collectable::LastCID;
    }

    inline bool Collectable::IsManaged()
    {
        if (this->_collectableManaged)
        {
            return true;
        }
        return this->HasSomeConsumers();
    }
}
#endif // COLLECTABLE_H
