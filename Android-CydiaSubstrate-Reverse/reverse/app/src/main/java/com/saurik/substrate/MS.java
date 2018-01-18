package com.saurik.substrate;

import java.lang.reflect.Constructor;
import java.lang.reflect.Member;
import java.lang.reflect.Method;

public final class MS {

    public interface ClassLoadHook {
        void classLoaded(Class<?> cls);
    }

    public interface MethodHook<T, R> {
        R invoked(T t, Object... objArr) throws Throwable;
    }

    public static abstract class MethodAlteration<T, R> implements MethodHook<T, R> {
        private final MethodPointer<T, R> method_ = new MethodPointer();

        public final R invoke(T thiz, Object... args) throws Throwable {
            return (R)this.method_.method_.invoke(thiz, args);
        }
    }

    public static final class MethodPointer<T, R> {
        private final com.saurik.substrate._MS.MethodPointer method_ = new com.saurik.substrate._MS.MethodPointer();

        public final R invoke(T thiz, Object... args) throws Throwable {
            return (R)this.method_.invoke(thiz, args);
        }
    }

    public static void hookClassLoad(String name, final ClassLoadHook hook) {
        _MS.hookClassLoad(name, new com.saurik.substrate._MS.ClassLoadHook() {
            public void classLoaded(Class clazz) {
                hook.classLoaded(clazz);
            }
        });
    }

    public static <T, R> void hookMethod_(Class clazz, Member member, final MethodHook<T, R> hook, MethodPointer<T, R> old) {
        com.saurik.substrate._MS.MethodPointer methodPointer = null;
        com.saurik.substrate._MS.MethodHook anonymousClass2 = hook != null ? new com.saurik.substrate._MS.MethodHook() {
            public R invoked(Object thiz, Object... args) throws Throwable {
                return hook.invoked((T) thiz, args);
            }
        } : null;
        if (old != null) {
            methodPointer = old.method_;
        }
        _MS.hookMethod(clazz, member, anonymousClass2, methodPointer);
    }

    public static <T, R> void hookMethod(Class clazz, Method member, MethodHook<T, R> hook, MethodPointer<T, R> old) {
        hookMethod_(clazz, member, hook, old);
    }

    public static <T, R> void hookMethod(Class clazz, Constructor member, MethodHook<T, R> hook, MethodPointer<T, R> old) {
        hookMethod_(clazz, member, hook, old);
    }

    private static <T, R> void hookMethod_(Class clazz, Member member, MethodAlteration<T, R> alteration) {
        hookMethod_(clazz, member, alteration, alteration.method_);
    }

    public static <T, R> void hookMethod(Class clazz, Method member, MethodAlteration<T, R> alteration) {
        hookMethod_(clazz, member, alteration);
    }

    public static <T, R> void hookMethod(Class clazz, Constructor member, MethodAlteration<T, R> alteration) {
        hookMethod_(clazz, member, alteration);
    }

    public static <T> T moveUnderClassLoader(ClassLoader loader, T object) {
        return _MS.moveUnderClassLoader(loader, object);
    }
}