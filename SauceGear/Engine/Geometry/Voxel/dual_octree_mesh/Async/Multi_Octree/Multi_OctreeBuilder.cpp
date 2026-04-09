#include "Multi_OctreeBuilder.h"

#include "../../Data/DCNode.h" 
#include "Multi_ConstructLeaf.h"
#include "../../../Density/Density.h"
#include "../../../Density/DensityBrickCache.h"
#include "../../../../../World/WorldController.h" 
#include "../../../../../World/C_Thread.h"
#include "../../../../../World/ThreadDef.h"
#include "../../../../../Utils/Threads/JLOG.h"

namespace MultiBuilder {

    //-------------------------------------------------------------------
    // Structs and TYPEDEFS
    //-------------------------------------------------------------------  

    struct NodeTaskData
    {
        DCNode* node;
        BuildCxt* ctx;
        // Functions
        std::function<void(DCNode*)>    OnComplete;
        Task* TaskFFn; // Task Father Function
    };

    // FUNCTIONS
    bool NodeHasSurface2(vec3 min, float size, BuildCxt& ctx)
    {
        const float d = Density_Func(min + (size / 2.0f));
        const float surfaceNetThreshold = size * 2 * 2.25f;
        return std::abs(d) < surfaceNetThreshold;
    }

    // TASK - FINALIZE CHILDREN
    void FinalizeNodeTask(Task* task, void* data)
    {
        auto* d = (NodeTaskData*)data;
        DCNode* node = d->node;

        bool hasChildren = false;

        for (int i = 0; i < 8; i++)
        {
            if (node->children[i]) hasChildren = true;
        }

        if (!hasChildren)
        {
            delete node;
            d->node = nullptr;
        }

        delete d; // IMPORTANTE evitar leak
    }

    void BuildNodeTask(Task* task, void* data)
    {
        auto* d = (NodeTaskData*)data;
        DCNode* node = d->node;
        auto& ctx = *d->ctx;

        float minimumNode = BASE_CELL_SIZE * (1 << ctx.chunkLOD);
        if (node->size == minimumNode)
        {
            ConstructLeaf(node, ctx);
            return;
        }

        float childSize = node->size / 2;

        for (int i = 0; i < 8; i++)
        {
            vec3 min = node->min + (CHILD_MIN_OFFSETS_FLOAT[i] * childSize);

            if (!NodeHasSurface2(min, childSize, ctx))
                continue;

            DCNode* child = GetNodeArena().Allocate();;    //new (child) DCNode();

            child->size = childSize;
            child->min = min;
            child->type = Node_Internal;

            node->children[i] = child;

            // ----------------------------------------------------------
            // Task Setup  
            // ----------------------------------------------------------

            // * Child *
            // cria build do filho
            NodeTaskData* childData = new NodeTaskData{ child, &ctx, nullptr };
            // cria finalize do filho
            //Task* childFinalize = global_JobSystem.CreateTask(FinalizeNodeTask, childData);
            // Fill Finalize Task
            //childData->TaskFatherFinalize = childFinalize;
            // e por fim cria a task do filho
            Task* childBuild = global_JobSystem.CreateTask(BuildNodeTask, childData);

            // * Schedule *
            // [dependency] build child -> finalize child
            //global_JobSystem.AddDependency(childBuild, childFinalize);
            // [dependency] finalize child -> finalize father 
            //global_JobSystem.AddDependency(childFinalize, d->TaskFatherFinalize);
            // childBuild -> childFinalize -> parentFinalize
            global_JobSystem.TrySchedule(childBuild);
        }

    }

    void BuildNodeSerial(DCNode* node, BuildCxt& ctx, int depth) {
        //JOB_LOG("[BUILD NODE] NODE " << node);
        if (node == nullptr) return;

        float minimumNode = BASE_CELL_SIZE * (1 << ctx.chunkLOD);
        if (node->size == minimumNode)
        {
            ConstructLeaf(node, ctx);

            //if (ConstructLeaf(node, ctx)) node->childMask.fetch_and(1 << i, std::memory_order_relaxed);
            
            return;
        }

        float childSize = node->size / 2;

        for (int i = 0; i < 8; i++)
        {
            vec3 min = node->min + (CHILD_MIN_OFFSETS_FLOAT[i] * childSize);

            if (!NodeHasSurface2(min, childSize, ctx))
                continue;

            DCNode* child = GetNodeArena().Allocate();;       //new (child) DCNode();
            MarkChildLife(node, i);         // node->childMask.fetch_or(1 << i, std::memory_order_relaxed);

            child->size = childSize;
            child->min = min;
            child->type = Node_Internal;

            node->children[i] = child;

            // MIN_TASK_SPLIT_DEPTH -> specify the minimum depth the node needs to be able to use tasks
            // TASK_SIZE_THRESHOLD  -> 
            float minNodeSizeToBeTask = (1 << ctx.chunkLOD) * TASK_SIZE_THRESHOLD;              //    child->size > TASK_SIZE_THRESHOLD
            if (child->size > minNodeSizeToBeTask && depth >= MIN_TASK_SPLIT_DEPTH)     //if ((child->size <= TASK_SIZE_THRESHOLD) || (depth < MIN_TASK_SPLIT_DEPTH) )    // *smallWork* or *tooEarly* 
            {
                NodeTaskData* data = new NodeTaskData{ child, &ctx, nullptr };

                Task* t = global_JobSystem.CreateTask(
                    [](Task*, void* ptr)
                    {
                        auto* d = (NodeTaskData*)ptr;
                        BuildNodeSerial(d->node, *d->ctx, MIN_TASK_SPLIT_DEPTH + 1);
                        delete d;
                    },
                    data
                );
                global_JobSystem.TrySchedule(t);
            }
            else BuildNodeSerial(child, ctx, depth + 1);     // continua inline (barato) 

        }
    }

    void BuildOctreeParallel(
        const ivec3 min,
        const int size,
        BuildCxt* ctx,
        std::function<void(DCNode*)> onComplete
    ) {
        // new Root

        DCNode* root = GetNodeArena().Allocate();;     //new (root) DCNode();                //ctx->memory->nodeArena.Allocate()

        root->min = min;
        root->size = size;
        root->type = Node_Internal;


        // TASK - ROOT FINALIZE (the last run -> ROOT, After its children) 
        Task* rootTask = global_JobSystem.CreateTask(
            // ~Lambda
            [](Task*, void* data) {
                auto* payload = (NodeTaskData*)data;

                DCNode* root = payload->node;
                BuildCxt* ctx = payload->ctx;

                //to run the final function after all its children have executed
                //auto& onComplete = payload->OnComplete;      

                BuildNodeSerial(root, *ctx, 0);

                //if (onComplete) onComplete(root);

                //delete payload; // cleanup
            },
            // ~Data 
            new NodeTaskData{ root, ctx, nullptr/*onComplete*/, nullptr }
        );

        global_JobSystem.TrySchedule(rootTask);
    }

}








/*

void BuildOctreeParallel(
    const ivec3 min,
    const int size,
    BuildCxt* ctx,
    std::function<void(DCNode*)> onComplete
) {

    BuildOctreeParallel2(min, size, ctx, onComplete);

    ctx->memory = new ChunkMemory(&global_NodePool);

    // new Root
    DCNode* root = ctx->memory->nodeArena.Allocate();
    new (root) DCNode();

    root->min = min;
    root->size = size;
    root->type = Node_Internal;

    // TASK - ROOT FINALIZE (the last run -> ROOT, After its children)
    Task* rootFinalize = jobSystem.CreateTask(
        // ~Lambda
        [](Task*, void* data) {
            auto* payload = (RootFinalizeData*)data;

            DCNode* root = payload->first;
            auto& onComplete = payload->second;

            if (onComplete) onComplete(root);

            delete payload; // cleanup
        },
        // ~Data
        new RootFinalizeData(root, onComplete)
    );

    //  ROOT BUILD - DATA
    NodeTaskData* rootData = new NodeTaskData{ root, ctx, rootFinalize };   // Here we propagate the (Task)rootFinalize to its children

    // Task Start with Data of the ROOT
    Task* rootBuild = jobSystem.CreateTask(BuildNodeTask, rootData);   // first Node to Start its children

    // build root -> finalize root
    //jobSystem.AddDependency(rootBuild, rootFinalize);

    jobSystem.TrySchedule(rootBuild);
}


*/










/*
Regras importantes para não dar problema
Não adicione jobs dependentes enquanto o job pai já está executando
Ou seja, sempre construa o grafo antes de rodar, ou use mutex/thread-safe containers para continuations.
*/


/*

    if (!createdChild)
    {
        if (d->TaskFatherFinalize)
        {
            if (d->TaskFatherFinalize->dependencies.fetch_sub(1) == 1)
            {
                global_JobSystem.TrySchedule(d->TaskFatherFinalize);
            }
        }
    }
*/


/*

    static bool p = true;
    if (p) {
        Task* rootBuild1 = global_JobSystem.CreateTask([](Task*, void* data) {  std::cout << "\n\n\n 1 TASK RUN " << std::endl;  }, nullptr);
        Task* rootBuild2 = global_JobSystem.CreateTask([](Task*, void* data) {  std::cout << " 2 TASK RUN " << std::endl;  }, nullptr);
        Task* rootBuild3 = global_JobSystem.CreateTask([](Task*, void* data) {  std::cout << " 3 TASK RUN \n\n\n" << std::endl;  }, nullptr);
        global_JobSystem.AddDependency(rootBuild1, rootBuild2);
        global_JobSystem.AddDependency(rootBuild2, rootBuild3);
        global_JobSystem.TrySchedule(rootBuild1);
        p = false;
    }

    return;
*/