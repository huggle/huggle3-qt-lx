== Garbage collector ==

This document describes the garbage collector used by huggle and how it works.

=== Preamble ===

This is an open document, feel free to improve it.

=== What is it ===

Garbage collector is specifically designed for lazy developers who don't want to keep track of resources. It allows collection (removal of resources) from operating memory. The point of GC is that it can collect the resources which are no longer needed for anything without requiring developers to explicitly delete them.

GC is basically a fundamental background job that is started by huggle, which check all managed objects and remove these, which aren't needed anymore from operating memory.

=== Why we need it ===

Because we are lazy as many other devs. This thing makes lot of things easier.

=== How does it work ===

Huggle uses its own GC which is very transparent and adaptive so it can be used as a drop in replacement for any other memory management. The garbage collector does only a collection of managed collectable objects. Every object that inherits class Huggle::Collectable is collectable. However, object which is collectable doesn't necessarily need to be removed by garbage collector unless it is managed. You must never call delete on any managed object, since that will raise an exception. Not all objects in huggle, in fact most of them aren't, collectable nor managed. That means you should use delete keyword for most of objects you work with, unless they are collectable. You always need to verify this, it's really important, but keep in mind, that devs are using GC only for objects where memory management is really complicated. Typical example of managed object is a query. These objects contain lot of dependecies and it's hard to determine whether they can be delete from memory or not, so that's why we are using garbage collector.
