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

#include "definitions.hpp"

namespace Huggle
{
    class Collectable;
    template <class T>
    //! This smart pointer can be used for any work with all kinds of collectable functions

    //! Unlike std smart pointers that are implemented in c++11 these are compatible with our GC
    //! which gives us bigger flexibility and higher performance.

    //! These ptrs don't need to allocate new manager for every single object that is maintained
    //! by any smart ptr like std ones do, but utilize the single instance GC we use
    class Collectable_SmartPtr
    {
        public:
            HUGGLE_EX Collectable_SmartPtr();
            HUGGLE_EX Collectable_SmartPtr(Collectable_SmartPtr *smart_ptr);
            HUGGLE_EX Collectable_SmartPtr(const Collectable_SmartPtr &sp_);
            HUGGLE_EX Collectable_SmartPtr<T>(T *pt);
            HUGGLE_EX virtual ~Collectable_SmartPtr();
            //! Change a bare pointer to other pointer
            HUGGLE_EX virtual void SetPtr(T *pt);
            //! Remove a bare pointer if there is some
            void Delete();
            void operator=(T* _ptr)
            {
                this->SetPtr(_ptr);
            }
            void operator=(const Collectable_SmartPtr &smart_ptr)
            {
                this->SetPtr(smart_ptr.GetPtr());
            }
            void operator=(std::nullptr_t &null)
            {
                this->SetPtr(null);
            }
            operator void* () const;
            HUGGLE_EX operator T* () const;
            HUGGLE_EX T* operator->();
            HUGGLE_EX T* GetPtr() const;
        protected:
        private:
            void FreeAcqRsrPtr();
            T *ptr;
    };

    template <class H>
    inline void Collectable_SmartPtr<H>::Delete()
    {
        this->FreeAcqRsrPtr();
    }

    template <class H>
    bool operator==(Collectable_SmartPtr<H> &smart_ptr, Collectable_SmartPtr<H> &smart_ptx)
    {
        return smart_ptr.GetPtr() == smart_ptx.GetPtr();
    }

    template <class H>
    bool operator!=(Collectable_SmartPtr<H> &smart_ptr, Collectable_SmartPtr<H> &smart_ptx)
    {
        return smart_ptr.GetPtr() != smart_ptx.GetPtr();
    }

    template <class H>
    bool operator==(Collectable_SmartPtr<H> &smart_ptr, std::nullptr_t ptr)
    {
        return smart_ptr.GetPtr() == ptr;
    }

    template <class H>
    bool operator!=(Collectable_SmartPtr<H> &smart_ptr, std::nullptr_t ptr)
    {
        return smart_ptr.GetPtr() != ptr;
    }

    template <class H>
    bool operator==(Collectable_SmartPtr<H> &smart_ptr, H* ptr)
    {
        return smart_ptr.GetPtr() == ptr;
    }

    template <class H>
    bool operator!=(Collectable_SmartPtr<H> &smart_ptr, H* ptr)
    {
        return smart_ptr.GetPtr() != ptr;
    }
}

#endif // COLLECTABLE_SMARTPTR_HPP
