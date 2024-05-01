Garbage collector
===================

This document describes the garbage collector used by Huggle and how it works.

Preamble
==========

This is an open document, feel free to improve it.

What is it
===========

Garbage collector is specifically designed for lazy developers, who don't want to keep track of resources. It allows for collection (removal of resources) from operating memory. The point of GC is that it can collect the resources which are no longer needed for anything, without requiring developers to explicitly delete them.

GC in Huggle is implemented as a background job that is started by Huggle core, which checks all managed objects and removes unneeded ones from operating memory.

Why we need it
================

Because many developers are lazy and this thing makes our work easier.

How does it work
==================

Huggle uses its own GC which is very transparent and adaptive, so it can be used as a drop in replacement for any other memory management. The garbage collector does only a collection of managed collectable objects. Every object that inherits class `Huggle::Collectable is collectable`. However, object which is collectable doesn't necessarily need to be removed by garbage collector unless it is managed. You must never call delete on any managed object, since that will raise an exception. Not all objects in Huggle (in fact most of them) are collectables or managed. That means you should use a delete keyword for most of objects you work with, unless they are collectable. You always need to verify this. It's really important, but keep in mind that devs are using GC only for objects where memory management is complicated. An example of a managed object is a query. These objects contain lot of dependecies and it's difficult to determine whether they can be deleted from memory or not, so that's why we are using garbage collector.

Collectable object
====================

When we use the term, "collectable object", we mean an object that can be collected by GC. In Huggle, all of these objects are derived from the Collectable class.

Managed object
================

Managed collectable is object that MUST be collected by GC. Deletion of managed objects result in exception.

Smart pointers
================

Modern implementation of GC supports smart pointers, which are the preferred and easiest way of using GC. The main disadvantage of smart pointers is that it's not easily possible to track down why some resources aren't being freed, so they are worse from a debugging point of view. Other than that, they are much easier to use than the traditional consumer model.

Consumers
============

Consumers are virtual elements that are using the collectable object. For example if you hold a reference to an object within some function, that function is a consumer of the object. The same applies for classes or fundamental pointers. Every consumer needs to be registered in collectable object so that GC knows it is still being used by it. When you no longer need to use some resource within a consumer, you need to remove it from collectable object.

In simple words, the consumers are locks that prevent GC from removing an object from memory.

How to create and collect managed object
=========================================

Every collectable object can be set to managed by registering at least one consumer. You can do that by calling `Huggle::Collectable::RegisterConsumer(QString)` or `Huggle::Collectable::RegisterConsumer(int)`. Once at least one consumer is registered, the object is managed. If all consumers of managed object are removed, the object is collected.
