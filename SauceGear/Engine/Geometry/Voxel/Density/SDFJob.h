#pragma once
#include "../../../Utils/Threads/JobSystem.h"
#include <glm/glm.hpp>

/*
struct SDFData
{
    float* sdf;
    glm::vec3* positions;
};

void SDFJob(Job* job, void* ptr)
{
    auto* data = (SDFData*)ptr;
    auto* batch = (JobSystem::BatchData*)job->data;

    for (int i = batch->start; i < batch->end; i++)
    {
        data->sdf[i] = SampleSDF(data->positions[i]);
    }

    delete batch;
}
*/


/*
auto counter = jobSystem.Dispatch(voxelCount, 64, SDFJob, &data);
jobSystem.Wait(counter);
*/