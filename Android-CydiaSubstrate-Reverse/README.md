# CydiaSubstrate_OpenSource
reverse and recover the code logic of Cydia Substrate on Android

Cydia Substrate is a hook library for android and ios device
It's easy to anaylysis an app with this tool, while not Open-Source
I'm doing reverse-engineering on it, and will soon recover it's main code

relevant files:

substrate.h             //c++ header file used in JNI layer hook 

substrate-api.jar       //import package used in java layer hook

substrate-bless.jar     //used to remove properties(private,protect,etc...) in java layer hook
com.saurik.substrate.apk//host apk, we can only develop plugin for it to install package

\lib\armeabi  \lib\x86  //real operation for hooking

libAndroidBootstrap0.so //used to fake /system/lib/liblog.so and pull up libAndroidLoader.so    each java process need to load liblog.so

libAndroidLoader.so     //used to pull all *.cy.so

libAndroidCydia.cy.so   //still in research

libDalvikLoader.cy.so   //still in research

libsubstrate.so         //provide jni layer hook low-level api

libsubstrate-dvm.so     //provide java layer hook low-level api

libSubstrateJNI.so      //used by substrate.apk to do c++ layer work

libSubstrateRun.so      //used by substrate.apk to do patch/unpatch/link/unlink operation

update-binary.so        //used by substrate.apk to recover patch/link operation
