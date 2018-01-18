package com.saurik.substrate;

import java.lang.reflect.Member;

final class _MS
{
    static native void hookClassLoad(String paramString, ClassLoadHook paramClassLoadHook);

    static native void hookMethod(Class paramClass, Member paramMember, MethodHook paramMethodHook, MethodPointer paramMethodPointer);

    static native <T> T moveUnderClassLoader(ClassLoader paramClassLoader, T paramT);

    static abstract interface ClassLoadHook
    {
        public abstract void classLoaded(Class paramClass);
    }

    static abstract interface MethodHook
    {
        public abstract Object invoked(Object paramObject, Object[] paramArrayOfObject)
                throws Throwable;
    }

    static final class MethodPointer
    {
        private long method;
        private Class<?>[] signature;
        private Class<?> type;

        public native Object invoke(Object paramObject, Object[] paramArrayOfObject)
                throws Throwable;
    }
}