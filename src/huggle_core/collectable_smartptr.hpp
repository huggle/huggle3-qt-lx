// IMPORTANT: this file has a different license than rest of huggle

//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2014 - 2018

#ifndef COLLECTABLE_SMARTPTR_HPP
#define COLLECTABLE_SMARTPTR_HPP

#include "definitions.hpp"
#include "collectable.hpp"
#include "exception.hpp"

namespace Huggle
{
    class Collectable;
    template <class T>
    //! This smart pointer can be used for any work with all kinds of collectable functions

    //! Unlike std smart pointers that are implemented in c++11 these are compatible with our GC
    //! which gives us bigger flexibility and higher performance.

    //! These ptrs don't need to allocate new manager for every single object that is maintained
    //! by any smart ptr like std ones do, but utilize the single instance GC we use
    class HUGGLE_EX_CORE Collectable_SmartPtr
    {
        public:
            Collectable_SmartPtr()
            {
                this->ptr = nullptr;
            }
            Collectable_SmartPtr(Collectable_SmartPtr *smart_ptr)
            {
                // we must not just copy the bare pointer but also increment the reference count
                if (smart_ptr->ptr)
                    smart_ptr->ptr->IncRef();
                this->ptr = smart_ptr->ptr;
            }
            Collectable_SmartPtr(const Collectable_SmartPtr &sp_)
            {
                if (sp_.ptr)
                    sp_.ptr->IncRef();
                this->ptr = sp_.ptr;
            }
            Collectable_SmartPtr<T>(T *pt)
            {
                if (!pt)
                {
                    this->ptr = nullptr;
                    return;
                }
                this->ptr = pt;
                this->ptr->IncRef();
            }
            virtual ~Collectable_SmartPtr()
            {
                this->FreeAcqRsrPtr();
            }
            //! Change a bare pointer to other pointer
            virtual void SetPtr(T *pt)
            {
                this->FreeAcqRsrPtr();
                if (pt)
                {
                    pt->IncRef();
                    this->ptr = pt;
                }
            }
            //! Remove a bare pointer if there is some
            void Delete()
            {
                this->FreeAcqRsrPtr();
            }
            Collectable_SmartPtr& operator=(T* _ptr)
            {
                this->SetPtr(_ptr);
                return *this;
            }
            Collectable_SmartPtr& operator=(const Collectable_SmartPtr &smart_ptr)
            {
                this->SetPtr(smart_ptr.GetPtr());
                return *this;
            }
            Collectable_SmartPtr& operator=(std::nullptr_t &null)
            {
                this->SetPtr(null);
                return *this;
            }
            operator void* () const
            {
                return this->ptr;
            }
            operator T* () const
            {
                return this->ptr;
            }
            T* operator->()
            {
                return this->ptr;
            }
            T* GetPtr() const
            {
                return this->ptr;
            }
        protected:
        private:
            void FreeAcqRsrPtr()
            {
                if (this->ptr)
                {
                    this->ptr->DecRef();
                    this->ptr = nullptr;
                }
            }
            T *ptr;
    };

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
