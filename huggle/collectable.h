//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.


#ifndef COLLECTABLE_H
#define COLLECTABLE_H

#include <QMutex>
#include <QList>
#include <QString>
#include <QStringList>
#include "gc.h"
#include "exception.h"

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
    class Collectable
    {
    public:
        Collectable();
        virtual ~Collectable();
        /*!
         * \brief IsManaged Managed class is deleted by GC and must not be deleted by hand
         * \return whether the class is managed
         */
        bool IsManaged();
        //! Use this if you are not sure if you can delete this object in this moment
        virtual bool SafeDelete();
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
        void RegisterConsumer(const int consumer);
        /*!
         * \brief This function will remove a string which prevent the object from being removed
         * \param consumer Unique string that unlock the object
         */
        void UnregisterConsumer(const int consumer);
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
         * \brief DebugHgc
         * \return debug info
         */
        QString DebugHgc();
        /*!
         * \brief CollectableID
         * \return
         */
        unsigned long CollectableID();
    private:
        static QString ConsumerIdToString(const int id);
        static QMutex WideLock;
        static unsigned long LastCID;
        unsigned long CID;
        //! Internal variable that contains a cache whether object is managed
        bool Managed;
        void SetManaged();
        QStringList Consumers;
        QList<int> iConsumers;
        QMutex *QL;
        bool Locked;
    };
}
#endif // COLLECTABLE_H
