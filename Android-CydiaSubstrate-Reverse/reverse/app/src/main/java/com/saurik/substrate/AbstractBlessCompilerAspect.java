package com.saurik.substrate;

import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Pointcut;

@Aspect
public abstract class AbstractBlessCompilerAspect {
    @Pointcut
    abstract void getConstValue();

    @Pointcut
    abstract void isAccessible();

    @Pointcut
    abstract void needsAccess();

    @Around("getConstValue()")
    public Object getConstValue(ProceedingJoinPoint join) throws Throwable {
        return null;
    }

    @Around("isAccessible()")
    public boolean isAccessible(ProceedingJoinPoint join) {
        return true;
    }

    @Around("needsAccess()")
    public boolean needsAccess(ProceedingJoinPoint join) {
        return false;
    }
}