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

#ifndef COLLECTABLE_SMARTPTR_HPP
#define COLLECTABLE_SMARTPTR_HPP

namespace Huggle
{
    class Collectable;
    template <class T>
    //! This smart pointer can be used for any work with all kinds of collectable functions
    class Collectable_SmartPtr
    {
        public:
            Collectable_SmartPtr();
            Collectable_SmartPtr(Collectable_SmartPtr<T> *smart_ptr);
            Collectable_SmartPtr(const Collectable_SmartPtr<T> &sp_);
            Collectable_SmartPtr(T *pt);
            virtual ~Collectable_SmartPtr();
            virtual void SetPtr(T *pt);
            template <class H>
            friend bool operator==(Collectable_SmartPtr<H> &smart_ptr, Collectable_SmartPtr<H> &smart_ptx);
            template <class H>
            friend bool operator!=(Collectable_SmartPtr<H> &smart_ptr, Collectable_SmartPtr<H> &smart_ptx);
            Collectable_SmartPtr<T> operator=(T *smart_ptr);
            Collectable_SmartPtr<T> operator=(const Collectable_SmartPtr<T> &smart_ptr);
            template <class H>
            friend bool operator!=(Collectable_SmartPtr<H> &smart_ptr, std::nullptr_t ptr);
            template <class H>
            friend bool operator==(Collectable_SmartPtr<H> &smart_ptr, std::nullptr_t ptr);
            operator void* () const;
            operator T* () const;
            T* operator->();
            T* GetPtr() const;
        protected:
        private:
            void FreeAcqRsrPtr();
            T *ptr;
    };

    template <class H>
    bool operator==(Collectable_SmartPtr<H> &smart_ptr, std::nullptr_t ptr)
    {
        return ((void*)smart_ptr.GetPtr() == ptr);
    }

    template <class H>
    bool operator!=(Collectable_SmartPtr<H> &smart_ptr, std::nullptr_t ptr)
    {
        return ((void*)smart_ptr.GetPtr() != ptr);
    }
}

#endif // COLLECTABLE_SMARTPTR_HPP
