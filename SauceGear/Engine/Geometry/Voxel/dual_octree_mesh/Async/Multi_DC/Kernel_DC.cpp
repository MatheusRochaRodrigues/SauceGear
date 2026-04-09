#include"Multi_DCMeshBuilder.h" 
#include"../../Data/DCNode.h"  
#include"../../../../../Utils/Threads/JobSystem.h"  
#include"../../../../../World/C_Thread.h"  
#include"TypesMB.h"   

namespace MultiBuilder {  
	void GenerateMeshTask(Task*, void* ptr)
	{
		auto* d = (MeshTaskData*)ptr;
		d->chunk->Reserve();

		GenerateVertexIndices(d->node, d->chunk->vb);
		ContourCellProc(d->node, d->chunk->ib);

		delete d;
	}

	void ScheduleMeshTasks(DCNode* node, std::vector<MeshChunk*>& chunks)
	{
		if (!IsValidNode(node)) return;

		// Create task in large nodes
		if (node->size >= MESH_TASK_SIZE_THRESHOLD)
		{
			MeshChunk* chunk = new MeshChunk();
			chunks.push_back(chunk);

			auto* data = new MeshTaskData{ node, chunk };

			Task* t = global_JobSystem.CreateTask(GenerateMeshTask, data);
			global_JobSystem.TrySchedule(t);

			return;
		}
		// We only schedule nodes that will traverse sufficiently large subtrees.
		if (node->type == Node_Internal)
		{
			for (int i = 0; i < 8; i++)
				ScheduleMeshTasks(node->children[i], chunks);
		}
	}

	void GenerateMeshFromOctree_MultiThread(DCNode* root, VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer)
	{
		if (!IsValidNode(root)) return;

		std::cout << "ENTROU OC MESH DC" << std::endl;

		vertexBuffer.clear();
		indexBuffer.clear();

		std::vector<MeshChunk*> chunks;
		chunks.reserve(128);

		// schedule
		ScheduleMeshTasks(root, chunks);

		// wait WaitAllJobs();

		// merge
		//MergeMeshChunks(chunks, vb, ib);

		// cleanup
		for (auto* c : chunks) delete c;

	}



}


