//#pragma once
//
//void* libraryHandle;
//
//#ifdef _WIN32
//libraryHandle = LoadLibraryA("scripts.dll");
//auto createScript = (ScriptBehaviour * (*)())GetProcAddress((HINSTANCE)libraryHandle, "CreateScript");
//#else
//libraryHandle = dlopen("scripts.so", RTLD_LAZY);
//auto createScript = (ScriptBehaviour * (*)())dlsym(libraryHandle, "CreateScript");
//#endif
//
//ScriptBehaviour* script = createScript();
