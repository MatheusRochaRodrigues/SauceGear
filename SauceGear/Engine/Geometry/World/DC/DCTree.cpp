#include	"DCTree.h"
#include	"../Density/density.h" 

const int MATERIAL_AIR = 0;
const int MATERIAL_SOLID = 1;

const float QEF_ERROR = 1e-6f;
const int QEF_SWEEPS = 4;
 

const ivec3 CHILD_MIN_OFFSETS[] =
{
	// needs to match the vertMap from Dual Contouring impl
	ivec3( 0, 0, 0 ),
	ivec3( 0, 0, 1 ),
	ivec3( 0, 1, 0 ),
	ivec3( 0, 1, 1 ),
	ivec3( 1, 0, 0 ),
	ivec3( 1, 0, 1 ),
	ivec3( 1, 1, 0 ),
	ivec3( 1, 1, 1 ),
};

// ----------------------------------------------------------------------------
// data from the original DC impl, drives the contouring process

const int edgevmap[12][2] = 
{
	{0,4},{1,5},{2,6},{3,7},	// x-axis 
	{0,2},{1,3},{4,6},{5,7},	// y-axis
	{0,1},{2,3},{4,5},{6,7}		// z-axis
};

const int edgemask[3] = { 5, 3, 6 } ;

const int vertMap[8][3] = 
{
	{0,0,0},
	{0,0,1},
	{0,1,0},
	{0,1,1},
	{1,0,0},
	{1,0,1},
	{1,1,0},
	{1,1,1}
};

const int faceMap[6][4] = {{4, 8, 5, 9}, {6, 10, 7, 11},{0, 8, 1, 10},{2, 9, 3, 11},{0, 4, 2, 6},{1, 5, 3, 7}} ;
const int cellProcEdgeMask[6][5] = {{0,1,2,3,0},{4,5,6,7,0},{0,4,1,5,1},{2,6,3,7,1},{0,2,4,6,2},{1,3,5,7,2}} ;
 
const int edgeProcEdgeMask[3][2][5] = {
	{{3,2,1,0,0},{7,6,5,4,0}},
	{{5,1,4,0,1},{7,3,6,2,1}},
	{{6,4,2,0,2},{7,5,3,1,2}},
};

const int processEdgeMask[3][4] = {{3,2,1,0},{7,5,6,4},{11,10,9,8}} ;

// -------------------------------------------------------------------------------

DCNode* SimplifyOctree(DCNode* node, float threshold)
{
	if (!node) return NULL; 

	if (node->type != Node_Internal)  return node;	// can't simplify!

	svd::QefSolver qef;
	int signs[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
	int midsign = -1;
	int edgeCount = 0;
	bool isCollapsible = true;

	for (int i = 0; i < 8; i++)
	{
		node->children[i] = SimplifyOctree(node->children[i], threshold);
		if (node->children[i])
		{
			DCNode* child = node->children[i];
			if (child->type == Node_Internal)
			{
				isCollapsible = false;
			}
			else
			{
				qef.add(child->drawInfo->qef);

				midsign = (child->drawInfo->corners >> (7 - i)) & 1; 
				signs[i] = (child->drawInfo->corners >> i) & 1; 

				edgeCount++;
			}
		}
	}

	if (!isCollapsible)
	{
		// at least one child is an internal node, can't collapse
		return node;
	}

	svd::Vec3 qefPosition;
	qef.solve(qefPosition, QEF_ERROR, QEF_SWEEPS, QEF_ERROR);
	float error = qef.getError();

	// convert to glm vec3 for ease of use
	vec3 position(qefPosition.x, qefPosition.y, qefPosition.z);

	// at this point the masspoint will actually be a sum, so divide to make it the average
	if (error > threshold)
	{
		// this collapse breaches the threshold
		return node;
	}

	if (position.x < node->min.x || position.x > (node->min.x + node->size) ||
		position.y < node->min.y || position.y > (node->min.y + node->size) ||
		position.z < node->min.z || position.z > (node->min.z + node->size))
	{
		const auto& mp = qef.getMassPoint();
		position = vec3(mp.x, mp.y, mp.z);
	}

	// change the node from an internal node to a 'psuedo leaf' node
	OctreeDrawInfo* drawInfo = new OctreeDrawInfo;

	for (int i = 0; i < 8; i++)
	{
		if (signs[i] == -1)
		{
			// Undetermined, use centre sign instead
			drawInfo->corners |= (midsign << i);
		}
		else 
		{
			drawInfo->corners |= (signs[i] << i);
		}
	}

	drawInfo->averageNormal = vec3(0.f);
	for (int i = 0; i < 8; i++)
	{
		if (node->children[i])
		{
			DCNode* child = node->children[i];
			if (child->type == Node_Psuedo || 
				child->type == Node_Leaf)
			{
				drawInfo->averageNormal += child->drawInfo->averageNormal;
			}
		}
	}

	drawInfo->averageNormal = glm::normalize(drawInfo->averageNormal);
	drawInfo->position = position;
	drawInfo->qef = qef.getData();

	for (int i = 0; i < 8; i++)
	{
		DestroyOctree(node->children[i]);
		node->children[i] = nullptr;
	}

	node->type = Node_Psuedo;
	node->drawInfo = drawInfo;

	return node;
}










// ----------------------------------------------------------------------------

void GenerateVertexIndices(DCNode* node, VertexBuffer& vertexBuffer)
{
	if (!node) return; 

	if (node->type != Node_Leaf) {
		for (int i = 0; i < 8; i++)
		{
			GenerateVertexIndices(node->children[i], vertexBuffer);
		}
	}

	if (node->type != Node_Internal)
	{
		OctreeDrawInfo* d = node->drawInfo;
		if (!d)
		{
			printf("Error! Could not add vertex!\n");
			exit(EXIT_FAILURE);
		}

		d->index = vertexBuffer.size();
		vertexBuffer.push_back(Vertex{d->position, d->averageNormal});		//vertexBuffer.push_back(Vertex(d->position, d->averageNormal));
	}
}

// ----------------------------------------------------------------------------

void ContourProcessEdge(DCNode* node[4], int dir, IndexBuffer& indexBuffer)
{
	int minSize = 1000000;		// arbitrary big number
	int minIndex = 0;
	int indices[4] = { -1, -1, -1, -1 };
	bool flip = false;
	bool signChange[4] = { false, false, false, false };

	for (int i = 0; i < 4; i++)
	{
		const int edge = processEdgeMask[dir][i];
		const int c1 = edgevmap[edge][0];
		const int c2 = edgevmap[edge][1];

		const int m1 = (node[i]->drawInfo->corners >> c1) & 1;
		const int m2 = (node[i]->drawInfo->corners >> c2) & 1;

		if (node[i]->size < minSize)
		{
			minSize = node[i]->size;
			minIndex = i;
			flip = m1 != MATERIAL_AIR; 
		}

		indices[i] = node[i]->drawInfo->index;

		signChange[i] = 
			(m1 == MATERIAL_AIR && m2 != MATERIAL_AIR) ||
			(m1 != MATERIAL_AIR && m2 == MATERIAL_AIR);
	}

	if (signChange[minIndex])
	{
		if (!flip)
		{
			indexBuffer.push_back(indices[0]);
			indexBuffer.push_back(indices[1]);
			indexBuffer.push_back(indices[3]);

			indexBuffer.push_back(indices[0]);
			indexBuffer.push_back(indices[3]);
			indexBuffer.push_back(indices[2]);
		}
		else
		{
			indexBuffer.push_back(indices[0]);
			indexBuffer.push_back(indices[3]);
			indexBuffer.push_back(indices[1]);

			indexBuffer.push_back(indices[0]);
			indexBuffer.push_back(indices[2]);
			indexBuffer.push_back(indices[3]);
		}
	}
}

// ----------------------------------------------------------------------------

// ContourEdgeProc resolve UMA ARESTA compartilhada por 4 células
// Ele garante que todas estejam no MESMO nível de detalhe
// Se não estiverem, ele subdivide a aresta recursivamente
void ContourEdgeProc(DCNode* node[4], int dir, IndexBuffer& indexBuffer)
{
	// Se qualquer uma das 4 células não existe,
	// essa aresta não é válida
	if (!node[0] || !node[1] || !node[2] || !node[3])
	{
		return;
	}

	// ------------------------------------------------------------------
	// CASO BASE:
	// Se NENHUMA das 4 células é interna,
	// então todas estão no nível mais fino relevante
	// → podemos gerar triângulos com segurança
	if (node[0]->type != Node_Internal &&
		node[1]->type != Node_Internal &&
		node[2]->type != Node_Internal &&
		node[3]->type != Node_Internal)
	{
		// Aqui acontece a geração real de triângulos
		ContourProcessEdge(node, dir, indexBuffer);
	}
	else
	{
		// ------------------------------------------------------------------
		// CASO RECURSIVO:
		// Pelo menos uma das células é interna
		// Isso significa que a aresta real está "escondida"
		// dentro de subdivisões menores
		//
		// Uma aresta 3D, ao descer um nível da octree,
		// se divide em DUAS sub-arestas
		for (int i = 0; i < 2; i++)
		{
			DCNode* edgeNodes[4];

			// Para essa sub-aresta i:
			// quais filhos de cada célula participam?
			const int c[4] =
			{
				edgeProcEdgeMask[dir][i][0],
				edgeProcEdgeMask[dir][i][1],
				edgeProcEdgeMask[dir][i][2],
				edgeProcEdgeMask[dir][i][3],
			};

			// Para cada uma das 4 células ao redor da aresta:
			for (int j = 0; j < 4; j++)
			{
				// Se a célula já é folha (ou pseudo-leaf),
				// ela já está no nível mais fino possível
				if (node[j]->type == Node_Leaf ||
					node[j]->type == Node_Psuedo)
				{
					edgeNodes[j] = node[j];
				}
				else
				{
					// Caso contrário, descemos para o filho
					// que toca essa sub-aresta específica
					edgeNodes[j] = node[j]->children[c[j]];
				}
			}

			// Recursão:
			// Continua descendo até que TODAS as 4 células
			// estejam no mesmo nível local
			ContourEdgeProc(edgeNodes, edgeProcEdgeMask[dir][i][4], indexBuffer);
		}
	}
}


// ---------------------------------------------------------------------------- 
const int faceProcFaceMask[3][4][3] = {
	{{4,0,0},{5,1,0},{6,2,0},{7,3,0}},		// X		
	{{2,0,1},{6,4,1},{3,1,1},{7,5,1}},		// Y
	{{1,0,2},{3,2,2},{5,4,2},{7,6,2}}		// Z
};
 
const int faceProcEdgeMask[3][4][6] = {
	{{1,4,0,5,1,1},{1,6,2,7,3,1},{0,4,6,0,2,2},{0,5,7,1,3,2}},
	{{0,2,3,0,1,0},{0,6,7,4,5,0},{1,2,0,6,4,2},{1,3,1,7,5,2}},
	{{1,1,0,3,2,0},{1,5,4,7,6,0},{0,1,5,0,4,1},{0,3,7,2,6,1}}
}; 

// ContourFaceProc resolve diferenças de LOD ENTRE DUAS CÉLULAS que compartilham UMA FACE
// Ela NÃO gera triângulos diretamente (exceto via EdgeProc)
// Ela garante que TODAS as arestas internas dessa face sejam processadas no nível correto
void ContourFaceProc(DCNode* node[2], int dir, IndexBuffer& indexBuffer)
{
	// Se uma das células não existe, não há face válida
	if (!node[0] || !node[1])
		return;

	// Se ambas são folhas, essa face NÃO precisa ser subdividida
	// As arestas dela serão resolvidas diretamente no EdgeProc
	if (node[0]->type == Node_Internal || node[1]->type == Node_Internal)
	{
		// ------------------------------------------------------------------
		// ETAPA 1: SUBDIVIDIR A FACE EM 4 SUBFACES (caso exista LOD diferente)
		//
		// Uma face 3D, ao descer um nível na octree, vira 4 subfaces 2D
		// Precisamos tratar cada uma separadamente
		for (int i = 0; i < 4; i++)
		{
			DCNode* faceNodes[2];

			// Para a subface i e direção dir:
			// quais filhos do node[0] e node[1] realmente se tocam nessa subface?
			const int c[2] = {
				//Para essa subface i e direção dir, quais filhos do node[0] e node[1] se tocam?
				faceProcFaceMask[dir][i][0], // filho do node[0]
				faceProcFaceMask[dir][i][1]  // filho do node[1]
			};

			// Para cada lado da face:
			// - Se o nó já é folha -> ele é o mais fino possível
			// - Se é interno → descemos para o filho correto que toca essa subface
			for (int j = 0; j < 2; j++)
			{
				if (node[j]->type != Node_Internal)
					faceNodes[j] = node[j];
				else
					faceNodes[j] = node[j]->children[c[j]];
			}

			// Recursão:
			// Continua descendo até que AMBOS os lados estejam no mesmo nível local
			ContourFaceProc(faceNodes, faceProcFaceMask[dir][i][2], indexBuffer);
		}

		// ------------------------------------------------------------------
		// ETAPA 2: PROCESSAR AS 4 ARESTAS INTERNAS DESSA FACE
		//
		// Após dividir a face em subfaces, surgem 4 arestas internas
		// Essas arestas são onde os cracks realmente aparecem
		const int orders[2][4] =
		{
			{ 0, 0, 1, 1 }, // padrão 0 → dois nós do lado 0, dois do lado 1
			{ 0, 1, 0, 1 }  // padrão 1 → intercalado
		};

		for (int i = 0; i < 4; i++)
		{
			DCNode* edgeNodes[4];

			// Para essa aresta interna:
			// quais filhos participam dela?
			const int c[4] =	//obs : só será necessario essa informação quando a aresta nao esta no no mais fino pois é necessario saber quais nós tocaram essa edge
			{
				faceProcEdgeMask[dir][i][1],
				faceProcEdgeMask[dir][i][2],
				faceProcEdgeMask[dir][i][3],
				faceProcEdgeMask[dir][i][4],
			};

			// Escolhe qual padrão de lados usar (node[0] ou node[1])
			const int* order = orders[faceProcEdgeMask[dir][i][0]];

			// Para cada um dos 4 nós que cercam a aresta:
			for (int j = 0; j < 4; j++)
			{
				// Se o nó já é folha (ou pseudo-leaf),
				// usamos ele diretamente
				if (node[order[j]]->type == Node_Leaf ||
					node[order[j]]->type == Node_Psuedo)
				{
					edgeNodes[j] = node[order[j]];
				}
				else
				{
					// Caso contrário, descemos para o filho correto
					edgeNodes[j] = node[order[j]]->children[c[j]];
				}
			}

			// Agora temos exatamente 4 células corretas ao redor da aresta
			// EdgeProc irá gerar os triângulos sem cracks
			ContourEdgeProc(edgeNodes, faceProcEdgeMask[dir][i][5], indexBuffer);
		}
	}
}


// ---------------------------------------------------------------------------- ------------------------
// THIS VECTOR tell diferrent combinations inter nodes of her father
const int cellProcFaceMask[12][3] = { {0,4,0},{1,5,0},{2,6,0},{3,7,0},{0,2,1},{4,6,1},{1,3,1},{5,7,1},{0,1,2},{2,3,2},{4,5,2},{6,7,2} };
//0 = X, 1 = Y, 2 = Z	-> direction

// ContourCellProc percorre a octree e inicia todo o processo de geração de malha
void ContourCellProc(DCNode* node, IndexBuffer& indexBuffer)
{
	if (!node) return;

	// Apenas nós internos participam:
	// folhas só armazenam vértices
	if (node->type != Node_Internal)
		return;

	// ------------------------------------------------------------------
	// ETAPA 1: RECURSÃO
	// Garante que os níveis mais finos sejam processados primeiro
	for (int i = 0; i < 8; i++)
		ContourCellProc(node->children[i], indexBuffer);

	// ------------------------------------------------------------------
	// ETAPA 2: PROCESSAR AS FACES INTERNAS ENTRE OS FILHOS
	//
	// Um cubo tem 12 pares de filhos que compartilham UMA FACE interna
	for (int i = 0; i < 12; i++)
	{
		DCNode* faceNodes[2];

		// Cada entrada da máscara indica:
		// - dois filhos que se tocam por uma face
		// - a direção dessa face (X, Y ou Z)
		const int c[2] = {
			cellProcFaceMask[i][0],
			cellProcFaceMask[i][1]
		};

		faceNodes[0] = node->children[c[0]];
		faceNodes[1] = node->children[c[1]];

		// Resolve LOD e subdivisões nessa face interna
		ContourFaceProc(faceNodes, cellProcFaceMask[i][2], indexBuffer);
	}

	// ------------------------------------------------------------------
	// ETAPA 3: PROCESSAR AS 6 ARESTAS INTERNAS DO CUBO
	//
	// Essas são arestas onde todos os 4 nós estão no mesmo nível
	for (int i = 0; i < 6; i++)
	{
		DCNode* edgeNodes[4];

		const int c[4] =
		{
			cellProcEdgeMask[i][0],
			cellProcEdgeMask[i][1],
			cellProcEdgeMask[i][2],
			cellProcEdgeMask[i][3],
		};

		for (int j = 0; j < 4; j++)
			edgeNodes[j] = node->children[c[j]];

		// Geração direta de triângulos
		ContourEdgeProc(edgeNodes, cellProcEdgeMask[i][4], indexBuffer);
	}
}


// ----------------------------------------------------------------------------

vec3 ApproximateZeroCrossingPosition(const vec3& p0, const vec3& p1)
{
	// approximate the zero crossing by finding the min value along the edge
	float minValue = 100000.f;
	float t = 0.f;
	float currentT = 0.f;
	const int steps = 8;
	const float increment = 1.f / (float)steps;
	while (currentT <= 1.f)
	{
		const vec3 p = p0 + ((p1 - p0) * currentT);
		const float density = glm::abs(Density_Func(p));
		if (density < minValue)
		{
			minValue = density;
			t = currentT;
		}

		currentT += increment;
	}

	return p0 + ((p1 - p0) * t);
}

// ----------------------------------------------------------------------------

vec3 CalculateSurfaceNormal(const vec3& p)
{
	const float H = 0.001f;
	const float dx = Density_Func(p + vec3(H, 0.f, 0.f)) - Density_Func(p - vec3(H, 0.f, 0.f));
	const float dy = Density_Func(p + vec3(0.f, H, 0.f)) - Density_Func(p - vec3(0.f, H, 0.f));
	const float dz = Density_Func(p + vec3(0.f, 0.f, H)) - Density_Func(p - vec3(0.f, 0.f, H));

	return glm::normalize(vec3(dx, dy, dz));
}

// ----------------------------------------------------------------------------

DCNode* ConstructLeaf(DCNode* leaf)
{
	if (!leaf || leaf->size != 1)
	{
		return nullptr;
	}

	int corners = 0;
	for (int i = 0; i < 8; i++)
	{
		const ivec3 cornerPos = leaf->min + CHILD_MIN_OFFSETS[i];
		const float density = Density_Func(vec3(cornerPos));
		const int material = density < 0.f ? MATERIAL_SOLID : MATERIAL_AIR;
		corners |= (material << i);
	}

	if (corners == 0 || corners == 255)
	{
		// voxel is full inside or outside the volume
		delete leaf;
		return nullptr;
	}

	// otherwise the voxel contains the surface, so find the edge intersections
	const int MAX_CROSSINGS = 6;
	int edgeCount = 0;
	vec3 averageNormal(0.f);
	svd::QefSolver qef;

	for (int i = 0; i < 12 && edgeCount < MAX_CROSSINGS; i++)
	{
		const int c1 = edgevmap[i][0];
		const int c2 = edgevmap[i][1];

		const int m1 = (corners >> c1) & 1;
		const int m2 = (corners >> c2) & 1;

		if ((m1 == MATERIAL_AIR && m2 == MATERIAL_AIR) ||
			(m1 == MATERIAL_SOLID && m2 == MATERIAL_SOLID))
		{
			// no zero crossing on this edge
			continue;
		}

		const vec3 p1 = vec3(leaf->min + CHILD_MIN_OFFSETS[c1]);
		const vec3 p2 = vec3(leaf->min + CHILD_MIN_OFFSETS[c2]);
		const vec3 p = ApproximateZeroCrossingPosition(p1, p2);
		const vec3 n = CalculateSurfaceNormal(p);
		qef.add(p.x, p.y, p.z, n.x, n.y, n.z);

		averageNormal += n;

		edgeCount++;
	}

	svd::Vec3 qefPosition;
	qef.solve(qefPosition, QEF_ERROR, QEF_SWEEPS, QEF_ERROR);

	OctreeDrawInfo* drawInfo = new OctreeDrawInfo;
	drawInfo->position = vec3(qefPosition.x, qefPosition.y, qefPosition.z);
	drawInfo->qef = qef.getData();

	const vec3 min = vec3(leaf->min);
	const vec3 max = vec3(leaf->min + ivec3(leaf->size));
	if (drawInfo->position.x < min.x || drawInfo->position.x > max.x ||
		drawInfo->position.y < min.y || drawInfo->position.y > max.y ||
		drawInfo->position.z < min.z || drawInfo->position.z > max.z)
	{
		const auto& mp = qef.getMassPoint();
		drawInfo->position = vec3(mp.x, mp.y, mp.z);
	}

	drawInfo->averageNormal = glm::normalize(averageNormal / (float)edgeCount);
	drawInfo->corners = corners;

	leaf->type = Node_Leaf;
	leaf->drawInfo = drawInfo;

	return leaf;
}

// -------------------------------------------------------------------------------

DCNode* ConstructOctreeNodes(DCNode* node)
{
	if (!node)
	{
		return nullptr;
	}

	if (node->size == 1)
	{
		return ConstructLeaf(node);
	}
	
	const int childSize = node->size / 2;
	bool hasChildren = false;

	for (int i = 0; i < 8; i++)
	{
		DCNode* child = new DCNode;
		child->size = childSize;
		child->min = node->min + (CHILD_MIN_OFFSETS[i] * childSize);
		child->type = Node_Internal;

		node->children[i] = ConstructOctreeNodes(child);
		hasChildren |= (node->children[i] != nullptr);
	}

	if (!hasChildren)
	{
		delete node;
		return nullptr;
	}

	return node;
}

// -------------------------------------------------------------------------------

DCNode* BuildOctree(const ivec3& min, const int size, const float threshold)
{
	DCNode* root = new DCNode;
	root->min = min;
	root->size = size;
	root->type = Node_Internal;

	ConstructOctreeNodes(root);
	//root = SimplifyOctree(root, threshold);

	return root;
}

// ----------------------------------------------------------------------------

void GenerateMeshFromOctree(DCNode* node, VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer)
{
	if (!node) return; 

	vertexBuffer.clear();
	indexBuffer.clear();		//onde os triângulos finais serão escritos

	GenerateVertexIndices(node, vertexBuffer);
	ContourCellProc(node, indexBuffer);
}

// -------------------------------------------------------------------------------

void DestroyOctree(DCNode* node)
{
	if (!node)
	{
		return;
	}

	for (int i = 0; i < 8; i++)
	{
		DestroyOctree(node->children[i]);
	}

	if (node->drawInfo)
	{
		delete node->drawInfo;
	}

	delete node;
}

// -------------------------------------------------------------------------------