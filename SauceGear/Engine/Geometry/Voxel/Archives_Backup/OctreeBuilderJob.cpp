#include "OctreeBuilderJob.h"

#include "DCNode.h" 
#include "../Density/Density.h"
#include "ConstructLeaf.h"
#include "../../Voxel/Density/DensityBrickCache.h"
#include "../../../World/WorldController.h"
#include "../../../Utils/Threads/JobSystem.h"  
#include "../../../World/ThreadDef.h"

//-------------------------------------------------------------------
// Structs
//-------------------------------------------------------------------

static BlockPool<DCNode, 4096> global_NodePool;

// JOBSYSTEM
static JobSystem jobSystem; 
 
struct NodeTaskData
{
    DCNode* node;
    BuildCxt* ctx;
    Task* TaskFatherFinalize;
};

// TYPEDEFS
typedef std::pair<DCNode*, std::function<void(DCNode*)>> RootFinalizeData; 
 
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

        DCNode* child = ctx.memory->nodeArena.Allocate();
        new (child) DCNode();

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
        //Task* childFinalize = jobSystem.CreateTask(FinalizeNodeTask, childData);
        // Fill Finalize Task
        //childData->TaskFatherFinalize = childFinalize;
        // e por fim cria a task do filho
        Task* childBuild = jobSystem.CreateTask(BuildNodeTask, childData);

        // * Schedule *
        // [dependency] build child -> finalize child
        //jobSystem.AddDependency(childBuild, childFinalize);
        // [dependency] finalize child -> finalize father 
        //jobSystem.AddDependency(childFinalize, d->TaskFatherFinalize);
        // childBuild -> childFinalize -> parentFinalize
        jobSystem.TrySchedule(childBuild);
    }

} 

void BuildNodeSerial(DCNode* node, BuildCxt& ctx, int depth)
{
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

        DCNode* child = ctx.memory->nodeArena.Allocate();
        child->size = childSize;
        child->min = min;
        child->type = Node_Internal;

        node->children[i] = child;

        // 🔥 DECISÃO INTELIGENTE
        bool smallWork = (child->size <= TASK_SIZE_THRESHOLD);
        bool tooEarly = (depth < MIN_TASK_SPLIT_DEPTH);

        if (smallWork || tooEarly)
        {
            // ✅ continua inline (barato)
            BuildNodeSerial(child, ctx, depth + 1);
        }
        else
        {
            // ✅ vira task (vale a pena)
            NodeTaskData* data = new NodeTaskData{ child, &ctx, nullptr };

            Task* t = jobSystem.CreateTask(
                [](Task*, void* ptr)
                {
                    auto* d = (NodeTaskData*)ptr;

                    BuildNodeSerial(d->node, *d->ctx, MIN_TASK_SPLIT_DEPTH + 1);

                    delete d;
                },
                data
            );

            jobSystem.TrySchedule(t);
        }
    }
}

void BuildOctreeParallel2(
    const ivec3 min,
    const int size,
    BuildCxt* ctx,
    std::function<void(DCNode*)> onComplete
) {
    DCNode* root = ctx->memory->nodeArena.Allocate();
    root->min = min;
    root->size = size;
    root->type = Node_Internal; 

    struct f { DCNode* n; BuildCxt* ck; 

    f(DCNode* node, BuildCxt* ctx) : n(node), ck(ctx) {}
    
    };

    typedef std::pair<f, std::function<void(DCNode*)>> df;


    // TASK - ROOT FINALIZE (the last run -> ROOT, After its children) 

    Task* rootTask = jobSystem.CreateTask(
        [](Task*, void* data) {
            auto* payload = (df*)data;

            f& dataPair = payload->first;
            DCNode* root = dataPair.n;
            BuildCxt* ctx = dataPair.ck;

            auto& onComplete = payload->second;

            BuildNodeSerial(root, *ctx, 0);

            if (onComplete) onComplete(root);

            delete payload;
        },
        new df({ root, ctx }, onComplete));

    jobSystem.TrySchedule(rootTask);
}

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
                jobSystem.TrySchedule(d->TaskFatherFinalize);
            }
        }
    }
*/


/*

    static bool p = true;
    if (p) {
        Task* rootBuild1 = jobSystem.CreateTask([](Task*, void* data) {  std::cout << "\n\n\n 1 TASK RUN " << std::endl;  }, nullptr);
        Task* rootBuild2 = jobSystem.CreateTask([](Task*, void* data) {  std::cout << " 2 TASK RUN " << std::endl;  }, nullptr);
        Task* rootBuild3 = jobSystem.CreateTask([](Task*, void* data) {  std::cout << " 3 TASK RUN \n\n\n" << std::endl;  }, nullptr);
        jobSystem.AddDependency(rootBuild1, rootBuild2);
        jobSystem.AddDependency(rootBuild2, rootBuild3);
        jobSystem.TrySchedule(rootBuild1);
        p = false;
    }

    return;
*/