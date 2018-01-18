package com.saurik.substrate;

import java.util.WeakHashMap;

import dalvik.system.PathClassLoader;

class SubstrateClassLoader extends PathClassLoader
{
    private WeakHashMap<ClassLoader, ClassLoader> loaders_;
    private String path_;

    private SubstrateClassLoader(String str, ClassLoader classLoader)
    {
        this(str, classLoader, new WeakHashMap());
        this.loaders_.put(classLoader, this);
    }

    private SubstrateClassLoader(String path, ClassLoader classLoader, WeakHashMap<ClassLoader, ClassLoader> weakHashMap)
    {
        super(path, classLoader);
        this.path_ = path;
        this.loaders_ = weakHashMap;
    }

    private ClassLoader get(ClassLoader loader)
    {
        ClassLoader cache = (ClassLoader)this.loaders_.get(loader);
        if (cache == null)
        {
            cache = new SubstrateClassLoader(this.path_, loader, this.loaders_);
            this.loaders_.put(loader, cache);
        }
        return cache;
    }

    private Object lock()
    {
        return this.loaders_;
    }
}