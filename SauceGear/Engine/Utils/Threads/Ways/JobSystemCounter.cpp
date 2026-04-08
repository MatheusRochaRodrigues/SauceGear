#include "JobSystem.h"

thread_local int JobSystem::threadIndex = -1;




//octree legacy 
/*
#include "OctreeBuilderJob.h"

#include "DCNode.h"
#include "../Density/Density.h"
#include "ConstructLeaf.h"
#include "../../Voxel/Density/DensityBrickCache.h"
#include "../../../World/WorldController.h"
#include "../../../Utils/Threads/JobSystem.h"

static JobSystem jobSystem;

bool NodeHasSurface2(vec3 min, float size, BuildContext_CK& ctx)
{
    const float d = Density_Func(min + (size / 2.0f));
    const float surfaceNetThreshold = size * 2 * 2.25f;
    return std::abs(d) < surfaceNetThreshold;
}

DCNode* BuildOctreeParallel(
    const ivec3 min,
    const int size,
    BuildContext_CK* ctx,
    std::function<void(DCNode*)> onComplete
    )
{
    DCNode* root = new DCNode;
    root->min = min;
    root->size = size;
    root->type = Node_Internal;

    auto* counter = jobSystem.CreateCounter
    (1, [root, onComplete]() {
        if (onComplete) onComplete(root);
    });

    JobData* data = new JobData{ root, ctx };

    Job job;
    job.function = BuildNodeJob;
    job.data = data;
    job.counter = counter;

    jobSystem.Schedule(job);

    return root;
}

void BuildNodeJob(Job* job, void* data)
{
    JobData* jd = (JobData*)data;
    DCNode* node = jd->node;
    BuildContext_CK& ctx = *jd->ctx;

    if (!node) return; //Caso a ser considerado para quando esta nulo

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

        DCNode* child = new DCNode;
        child->size = childSize;
        child->min = min;
        child->type = Node_Internal;

        node->children[i] = child;

        // cria job filho
        JobData* childData = new JobData{ child, &ctx };

        Job childJob;
        childJob.function = BuildNodeJob;
        childJob.data = childData;
        childJob.counter = job->counter;

        job->counter->value.fetch_add(1);           // IMPORTANTE
        jobSystem.Schedule(childJob);
    }
}





*/