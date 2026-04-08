#include "JobSystem.h"

//BlockPool<Task, 2048> global_TaskPool;


thread_local int JobSystem::threadIndex = -1;


/*
// THREADS
//extern BlockPool<Task, 2048> global_TaskPool;
//extern ThreadArena<Task, 2048> nodeArena;



BlockPool<Task, 2048> global_TaskPool;
ThreadArena<Task, 2048> nodeArena(&global_TaskPool);
*/


/* if necessary, each thread will have its own arena
thread_local ThreadArena<Task, 2048> nodeArena(&global_TaskPool);
*/




