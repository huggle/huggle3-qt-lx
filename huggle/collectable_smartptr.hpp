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
            Collectable_SmartPtr(Collectable_SmartPtr *smart_ptr);
            Collectable_SmartPtr(const Collectable_SmartPtr &sp_);
            Collectable_SmartPtr<T>(T *pt);
            virtual ~Collectable_SmartPtr();
            virtual void SetPtr(T *pt);
            void operator=(T* _ptr)
            {
                this->SetPtr(_ptr);
            }
            Collectable_SmartPtr& operator=(const Collectable_SmartPtr &smart_ptr)
            {
                if (smart_ptr.ptr != nullptr)
                    smart_ptr.ptr->IncRef();
                this->ptr = smart_ptr.ptr;
                return *this;
            }
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
}

#endif // COLLECTABLE_SMARTPTR_HPP
